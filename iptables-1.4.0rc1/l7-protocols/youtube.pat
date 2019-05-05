# youtube - Youtube over HyperText Transfer Protocol (RFC 2616)
# Pattern attributes: good notsofast fast
# Protocol groups: remote_access x_consortium_standard
# Wiki: http://www.protocolinfo.org/wiki/X11
#
# It is common for X to be tunneled through SSH.  Then obviously this pattern
# will not catch it.
#
# Specification: http://www.msu.edu/~huntharo/xwin/docs/xwindows/PROTO.pdf
# Usually runs on port 6000 (6001 for the second server on a host, etc)
#
# This pattern has been tested.

#
# 'l' = little-endian.  'B' = big endian
# ".?" is for the unused byte that comes next.  If it's a null, it won't appear.
# \x0b = protocol-major-version 11.
# For some reason, protocol-minor-version is 0, not 6, so can't match it.
# This pattern is too general. 
youtube
GET (\/videoplayback\?|\/crossdomain\.xml)
