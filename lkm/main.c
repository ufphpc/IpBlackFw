#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/types.h>
#include <linux/version.h>

#include "fw_policy.h"
#include "device_io.h"

static unsigned int fw_ip_check_route(struct sk_buff *skb)
{
	unsigned int ret = NF_ACCEPT;
	int offset, ihlen;
	struct iphdr _iph, *ih;

	offset = skb_network_offset(skb);
	ih = skb_header_pointer(skb, offset, sizeof(_iph), &_iph);
	if (ih == NULL)
		goto out;

	ihlen = ih->ihl * 4;
	if (ihlen < sizeof(_iph))
		goto out;

	if(fw_check_ip_rule(ih->saddr) == -1) {
		ret = NF_DROP;
	}

out:
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)

static unsigned int fw_ipv4_postroute(void* priv,
				struct sk_buff* skb,
				const struct nf_hook_state* state)
{
	return NF_ACCEPT;
}

#else

static unsigned int fw_ipv4_postroute(unsigned int hooknum,
	struct sk_buff* skb,
	const struct net_device* in,
	const struct net_device* out,
	int (*okfn)(struct sk_buff*))
{
	return NF_ACCEPT;
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)

static unsigned int fw_ipv4_pre_route(void* priv,
				struct sk_buff* skb,
				const struct nf_hook_state* state)
{
	return fw_ip_check_route(skb);
}

#else

static unsigned int fw_ipv4_pre_route(unsigned int hooknum,
	struct sk_buff* skb,
	const struct net_device* in,
	const struct net_device* out,
	int (*okfn)(struct sk_buff*))
{
	return fw_ip_check_route(skb);
}

#endif

static struct nf_hook_ops fw_hook_ops[] __read_mostly = {
	{
		.hook = fw_ipv4_postroute,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	},
	{
		.hook = fw_ipv4_pre_route,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	}
};

static int __init fw_init(void)
{
	int ret;

	ret = fw_policy_init();
	if (ret < 0) {
		goto out;
	}

	ret = device_io_init();
	if (ret < 0) {
		goto err_policy;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	ret = nf_register_net_hooks(&init_net, fw_hook_ops,  ARRAY_SIZE(fw_hook_ops));
#else
	ret = nf_register_hooks(fw_hook_ops,  ARRAY_SIZE(fw_hook_ops));
#endif

	if (ret < 0) {
		pr_err("IpBlackFw: fw_init register_hooks err[%d]\n", ret);
		goto err_device;
	}

	pr_info("IpBlackFw: fw_init ...\n");
	ret = 0;
	goto out;

err_device:
	device_io_uninit();
err_policy:
	fw_policy_cleanup();
out:
	return ret;
}

static void __exit fw_exit(void)
{
    pr_info("IpBlackFw: fw_exit ...\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	nf_unregister_net_hooks(&init_net, fw_hook_ops,  ARRAY_SIZE(fw_hook_ops));
#else
	nf_unregister_hooks(fw_hook_ops,  ARRAY_SIZE(fw_hook_ops));
#endif
	device_io_uninit();
	fw_policy_cleanup();
}

module_init(fw_init);
module_exit(fw_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiaorui zhang");