#!/usr/bin/perl
# Simple script for generating prototypes for C functions
# Written by Jelmer Vernooij
# based on the original mkproto.sh by Andrew Tridgell

use strict;

# don't use warnings module as it is not portable enough
# use warnings;

use Getopt::Long;
use File::Basename;
use File::Path;

#####################################################################
# read a file into a string

my $public_file = undef;
my $private_file = undef;
my $all_file = undef;
my $public_define = undef;
my $private_define = undef;
my $_public = "";
my $_private = "";
my $public_data = \$_public;
my $private_data = \$_private;
my $builddir = ".";
my $srcdir = ".";

sub public($)
{
	my ($d) = @_;
	$$public_data .= $d;
}

sub private($)
{
	my ($d) = @_;
	$$private_data .= $d;
}

sub usage()
{
	print "Usage: mkproto.pl [options] [c files]\n";
	print "OPTIONS:\n";
	print "  --public=FILE          Write prototypes for public functions to FILE\n";
	print "  --private=FILE         Write prototypes for private functions to FILE\n";
	print "  --define=DEF           Use DEF to check whether header was already included\n";
	print "  --public-define=DEF    Same as --define, but just for public header\n";
	print "  --private-define=DEF   Same as --define, but just for private header\n";
	print "  --srcdir=path          Read files relative to this directory\n";
	print "  --builddir=path        Write file relative to this directory\n";
	print "  --help                 Print this help message\n\n";
	exit 0;
}

GetOptions(
	'public=s' => sub { my ($f,$v) = @_; $public_file = $v; },
	'all=s' => sub { my ($f,$v) = @_; $public_file = $v; $private_file = $v; },
	'private=s' => sub { my ($f,$v) = @_; $private_file = $v; },
	'define=s' => sub { 
		my ($f,$v) = @_; 
		$public_define = $v; 
		$private_define = "$v\_PRIVATE"; 
	},
	'public-define=s' => \$public_define,
	'private-define=s' => \$private_define,
	'srcdir=s' => sub { my ($f,$v) = @_; $srcdir = $v; },
	'builddir=s' => sub { my ($f,$v) = @_; $builddir = $v; },
	'help' => \&usage
) or exit(1);

sub normalize_define($$)
{
	my ($define, $file) = @_;

	if (not defined($define) and defined($file)) {
		$define = "__" . uc($file) . "__";
		$define =~ tr{./}{__};
		$define =~ tr{\-}{_};
	} elsif (not defined($define)) {
		$define = '_PROTO_H_';
	}

	return $define;
}

$public_define = normalize_define($public_define, $public_file);
$private_define = normalize_define($private_define, $private_file);

if ((defined($private_file) and defined($public_file) and ($private_file eq $public_file)) or 
	(not defined($private_file) and not defined($public_file))) {
	$private_data = $public_data;
}

sub file_load($)
{
    my($filename) = @_;
    local(*INPUTFILE);
    open(INPUTFILE, $filename) or return undef;
    my($saved_delim) = $/;
    undef $/;
    my($data) = <INPUTFILE>;
    close(INPUTFILE);
    $/ = $saved_delim;
    return $data;
}

sub print_header($$)
{
	my ($file, $header_name) = @_;
	$file->("#ifndef $header_name\n");
	$file->("#define $header_name\n\n");
	$file->("#undef _PRINTF_ATTRIBUTE\n");
	$file->("#define _PRINTF_ATTRIBUTE(a1, a2) PRINTF_ATTRIBUTE(a1, a2)\n");
	$file->("/* This file was automatically generated by mkproto.pl. DO NOT EDIT */\n\n");
}

sub print_footer($$) 
{
	my ($file, $header_name) = @_;
	$file->("#undef _PRINTF_ATTRIBUTE\n");
	$file->("#define _PRINTF_ATTRIBUTE(a1, a2)\n");
	$file->("\n#endif /* $header_name */\n\n");
}

sub handle_loadparm($$) 
{
	my ($file,$line) = @_;
	my $scope;
	my $type;
	my $name;
	my $var;

	if ($line =~ /^FN_(GLOBAL|LOCAL)_(CONST_STRING|STRING|BOOL|bool|CHAR|INTEGER|LIST)\((\w+),(.*)\)/o) {
		$scope = $1;
		$type = $2;
		$name = $3;
		$var = $4;
	} elsif ($line =~ /^FN_(GLOBAL|LOCAL)_PARM_(CONST_STRING|STRING|BOOL|bool|CHAR|INTEGER|LIST)\((\w+),(.*)\)/o) {
		$scope = $1;
		$type = $2;
		$name = $3;
		$var = $4;
	} else {
	        return;
	}

	my %tmap = (
	        "BOOL" => "bool ",
	        "CONST_STRING" => "const char *",
	        "STRING" => "const char *",
	        "INTEGER" => "int ",
	        "CHAR" => "char ",
	        "LIST" => "const char **",
	);

	my %smap = (
	        "GLOBAL" => "struct loadparm_context *",
	        "LOCAL" => "struct loadparm_service *, struct loadparm_service *"
	        );

	$file->("$tmap{$type}lpcfg_$name($smap{$scope});\n");
}

sub process_file($$$) 
{
	my ($public_file, $private_file, $filename) = @_;

	$filename =~ s/\.o$/\.c/g;

	if ($filename =~ /^\//) {
		open(FH, "<$filename") or die("Failed to open $filename");
	} elsif (!open(FH, "< $builddir/$filename")) {
	    open(FH, "< $srcdir/$filename") || die "Failed to open $filename";
	}

	$private_file->("\n/* The following definitions come from $filename  */\n\n");

	my $comment = undef;
	my $incomment = 0;
	while (my $line = <FH>) {	      
		my $target = \&private;
		my $is_public = 0;

		if ($line =~ /^\/\*\*/) { 
			$comment = "";
			$incomment = 1;
		}

		if ($incomment) {
			$comment .= $line;
			if ($line =~ /\*\//) {
				$incomment = 0;
			}
		} 

		# these are ordered for maximum speed
		next if ($line =~ /^\s/);
	      
		next unless ($line =~ /\(/);

		next if ($line =~ /^\/|[;]/);

		if ($line =~ /^FN_/) {
			handle_loadparm($public_file, $line);
			handle_loadparm($private_file, $line);
			next;
		}

		if ($line =~ /^_PUBLIC_[\t ]/) {
			$target = \&public;
			$is_public = 1;
		}

		next unless ( $is_public || $line =~ /
			      ^(_DEPRECATED_ |_NORETURN_ |_WARN_UNUSED_RESULT_ |_PURE_ )*(
				  void|bool|int|struct|char|const|\w+_[tT]\s|uint|unsigned|long|NTSTATUS|
				  ADS_STATUS|enum\s.*\(|DATA_BLOB|WERROR|XFILE|FILE|DIR|
			      double|TDB_CONTEXT|TDB_DATA|TALLOC_CTX|NTTIME|FN_|init_module|
			      GtkWidget|GType|smb_ucs2_t|krb5_error_code|NET_API_STATUS)
			      /xo);

		next if ($line =~ /^int\s*main/);

		$target->("\n$comment") if (defined($comment)); $comment = undef;

		if ( $line =~ /\(.*\)\s*$/o ) {
			chomp $line;
			$target->("$line;\n");
			next;
		}

		$target->($line);

		while ($line = <FH>) {
			if ($line =~ /\)\s*$/o) {
				chomp $line;
				$target->("$line;\n");
				last;
			}
			$target->($line);
		}
	}

	close(FH);
}


print_header(\&public, $public_define);
if (defined($private_file) and defined($public_file) and $public_file ne $private_file) {
	print_header(\&private, $private_define);

	private("/* this file contains prototypes for functions that " .
			"are private \n * to this subsystem or library. These functions " .
			"should not be \n * used outside this particular subsystem! */\n\n");

	public("/* this file contains prototypes for functions that " . 
			"are part of \n * the public API of this subsystem or library. */\n\n");

}

public("#ifndef _PUBLIC_\n#define _PUBLIC_\n#endif\n\n");
public("#ifndef _PURE_\n#define _PURE_\n#endif\n\n");
public("#ifndef _NORETURN_\n#define _NORETURN_\n#endif\n\n");
public("#ifndef _DEPRECATED_\n#define _DEPRECATED_\n#endif\n\n");
public("#ifndef _WARN_UNUSED_RESULT_\n#define _WARN_UNUSED_RESULT_\n#endif\n\n");

process_file(\&public, \&private, $_) foreach (@ARGV);
print_footer(\&public, $public_define);
if (defined($private_file) and $public_file ne $private_file) {
	print_footer(\&private, $private_define);
}

if (not defined($public_file)) {
	print STDOUT $$public_data;
}

if (not defined($private_file) and defined($public_file)) {
	print STDOUT $$private_data;
}

mkpath(dirname($public_file), 0, 0755);
open(PUBLIC, ">$public_file") or die("Can't open `$public_file': $!"); 
print PUBLIC "$$public_data";
close(PUBLIC);

if (defined($private_file) and $public_file ne $private_file) {
	mkpath(dirname($private_file), 0, 0755);
	open(PRIVATE, ">$private_file") or die("Can't open `$private_file': $!"); 
	print PRIVATE "$$private_data";
	close(PRIVATE);
}
