/*
 * The Kernel Module
 *
 * Use "kbuild" to compile. Protocal in line 40. Port in line 42. Hook type in line 70.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#include <net/ip.h>

#define NIPQUAD(addr)   ((unsigned char *)&addr)[0],   ((unsigned char *)&addr)[1],   ((unsigned char *)&addr)[2],   ((unsigned char *)&addr)[3]
#define MAX_CLI  64
#define ULONG_LEN 4294800000 
#define NETLINK_STATFILTER	28

struct recStatClient {
	uint8_t ipAddr[24];
	unsigned long out;
	unsigned long in;
	unsigned long time;
};
struct recStatClient statClientList[MAX_CLI];
struct sock *nl_sk = NULL;
char g_data[4096] = {0};

static unsigned int hook_func(unsigned int hooknum,
                              struct sk_buff *skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
{
	struct ethhdr *ethh;
    struct iphdr *iph;
    struct udphdr *udph;
	__be32 sip,dip;
	uint8_t requestIp[24];
	uint8_t destIp[24];
	int i, flag;

	//printk("in name: %s, out name: %s\n", in->name, out->name);

    if (!skb) return NF_ACCEPT;
	
    iph = ip_hdr(skb);
	if(!iph) return NF_ACCEPT;
	
	if (iph->protocol != IPPROTO_UDP && iph->protocol != IPPROTO_TCP) return NF_ACCEPT;

	/*
	if (hooknum == NF_INET_PRE_ROUTING) {
		if(strncmp(in->name, "br0", strlen("br0")) != 0) return NF_ACCEPT;
	} else {
		if(strncmp(out->name, "br0", strlen("br0")) != 0) return NF_ACCEPT;
	}
	*/

	//udph = (struct udphdr *)((char *)skb->data + (int)(iph->ihl * 4));
	unsigned int datalen = ntohs(iph->tot_len);
	if (datalen == 0) return NF_ACCEPT;
	//unsigned int datalen = skb->data_len;
	//unsigned int len = skb->len;
	//printk("=====udpLen: %d, len: %d, data_len: %d\n======", udpLen, len, datalen);
	//printk("=====jiffies: %lu======\n", jiffies);
	
	sip = iph->saddr;
	dip = iph->daddr;
	sprintf(requestIp, "%u.%u.%u.%u", NIPQUAD(sip));
	sprintf(destIp, "%u.%u.%u.%u", NIPQUAD(dip));

	//broadcast
	dip = dip&0xff000000;
	if(ntohl(dip) == 255) return NF_ACCEPT;

	//printk("request IP: %s, dest IP: %s\n", requestIp, destIp);
	//struct net_device *LanInt = dev_get_by_name(&init_net, "br0");

	//TrRate: 流出流量
	flag = 0;
	if (hooknum == NF_BR_PRE_ROUTING) {
		//printk("RxRate requestIp: %s, datalen: %d\n", requestIp, datalen);
		for(i=0; i < MAX_CLI; i++) {
			if(0 == strcmp(statClientList[i].ipAddr, requestIp) || 0 == strlen(statClientList[i].ipAddr)) {
				sprintf(statClientList[i].ipAddr, "%s", requestIp);
				statClientList[i].out += datalen;
				statClientList[i].time = jiffies;
				break;
			}

			if (i > 0) {
				if (statClientList[i].time < statClientList[i-1].time) flag = i;
			}
		}

		if(i >= MAX_CLI) {
			sprintf(statClientList[flag].ipAddr, "%s", requestIp);
			statClientList[flag].out += datalen;
			statClientList[flag].time = jiffies;
		}

		if (statClientList[flag].out > ULONG_LEN) statClientList[flag].out = 0;
		
	}else {
		//RxRate : 流入流量
		//printk("TxRate destIp: %s, datalen :%d\n", destIp, datalen);
		for(i=0; i < MAX_CLI; i++) {
			if(0 == strcmp(statClientList[i].ipAddr, destIp) || 0 == strlen(statClientList[i].ipAddr)) {
				sprintf(statClientList[i].ipAddr, "%s", destIp);
				statClientList[i].in += datalen;
				statClientList[i].time = jiffies;
				break;
			}

			if (i > 0) {
				if (statClientList[i].time < statClientList[i-1].time) flag = i;
			}
		}

		if(i >= MAX_CLI) {
			sprintf(statClientList[flag].ipAddr, "%s", destIp);
			statClientList[flag].in += datalen;
			statClientList[flag].time = jiffies;
		}

		if (statClientList[flag].in > ULONG_LEN) statClientList[flag].in = 0;

	}

	return NF_ACCEPT;
}


static void stat_filter_netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	int pid, j, res, msg_size;
	struct sk_buff *skb_out;
	char *msg=(char *)g_data;

	memset(g_data,0,sizeof(g_data));
	for(j=0; j< MAX_CLI; j++) {
		if(strlen(statClientList[j].ipAddr) != 0) {
			char tempbuf[64] = {0};
			sprintf(tempbuf, "%s,%lu,%lu;", statClientList[j].ipAddr,
								statClientList[j].in, statClientList[j].out);
			strcat(g_data, tempbuf);
		}
	}
	
	g_data[strlen(g_data)] = '\0';

//	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	msg_size=strlen(msg);
	//for receiving...
	nlh=(struct nlmsghdr*)skb->data;	//nlh message comes from skb's data... (sk_buff: unsigned char *data)
	/*	static inline void *nlmsg_data(const struct nlmsghdr *nlh)
		{
			return (unsigned char *) nlh + NLMSG_HDRLEN;
		} 
	nlmsg_data - head of message payload */
//	printk(KERN_INFO "Netlink received msg payload: %s\n",(char*)nlmsg_data(nlh));
	
	//for sending...
	pid = nlh->nlmsg_pid; // Sending process port ID, will send new message back to the 'user space sender'
	
	skb_out = nlmsg_new(msg_size,0);	//nlmsg_new - Allocate a new netlink message: skb_out

	if(!skb_out)
	{
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}

	nlh=nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);	//nlmsg_put - Add a new netlink message to an skb
														//argument 1 - @skb: socket buffer to store message in
														//argument 2 - @pid = 0, from kernel
														//NLMSG_DONE - End of a dump
														//nlh points to skb_out now...

	//#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))
	//cb: This is the control buffer. It is free to use for every layer. Please put your private variables there
	/* struct netlink_skb_parms {
	struct ucred		creds;		// Skb credentials
	__u32			pid;
	__u32			dst_group;
	}; */
	//map skb->cb (char cb[48] __aligned(8); control buffer) to "struct netlink_skb_parms", so it has field pid/dst_group
	//so there should be convention: cb[48] is divided into creds/pid/dst_group...to convey those info
	NETLINK_CB(skb_out).dst_group = 0;					/* not in mcast group */
	strncpy(nlmsg_data(nlh),msg,msg_size);				//char *strncpy(char *dest, const char *src, size_t count)
														//msg "Hello from kernel" => nlh -> skb_out

	res=nlmsg_unicast(nl_sk,skb_out,pid);				//nlmsg_unicast - unicast a netlink message
														//@pid: netlink pid of the destination socket
	if(res<0)
		printk(KERN_INFO "Error while sending bak to user\n");
}


static struct nf_hook_ops br_nf_ops[] __read_mostly = {
	{
		.hook = hook_func,
		.owner = THIS_MODULE,
		.pf = PF_BRIDGE,
		.hooknum = NF_BR_PRE_ROUTING,
		.priority = NF_BR_PRI_FIRST + 1,
	},
	{
		.hook = hook_func,
		.owner = THIS_MODULE,
		.pf = PF_BRIDGE,
		.hooknum = NF_BR_POST_ROUTING,
		.priority = NF_BR_PRI_FIRST - 1,

	}
};


/*
 * Initialisation
 */
int init_module(void)
{
    printk(KERN_INFO "-----Stat Filter Module Start-----");
	memset(statClientList, 0, sizeof(statClientList));
	nf_register_hooks(br_nf_ops, ARRAY_SIZE(br_nf_ops));

	//创建netlink
	nl_sk=netlink_kernel_create(&init_net, NETLINK_STATFILTER, 0, stat_filter_netlink_rcv, NULL, THIS_MODULE);
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

/*
 * Cleanup
 */
void cleanup_module(void)
{
    printk(KERN_INFO "-----Stat Filter Module Stop-----");
    nf_unregister_hooks(br_nf_ops, ARRAY_SIZE(br_nf_ops));
    netlink_kernel_release(nl_sk);
}

MODULE_AUTHOR("xiaoning");
MODULE_DESCRIPTION("A kernel module which use netfilter hook to get designated incoming/outgoing HTTP packets.");
MODULE_LICENSE("GPL");
