#include <linux/kernel.h>
#include <linux/rbtree.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "fw_policy.h"

//block ip policy
struct fw_policy_node {
    struct rb_node node;
	__u32  ip;
};

static struct rb_root fw_policy_root = RB_ROOT;
static struct kmem_cache *fw_policy_cache __read_mostly = NULL;

//DEFINE_SPINLOCK(fw_policy_lock);

static DEFINE_RWLOCK(fw_policy_rwlock);

int fw_policy_init(void)
{
    int ret;

    fw_policy_cache = kmem_cache_create("fw_policy",
                            sizeof(struct fw_policy_node),
                            0, 0, NULL);
    if (!fw_policy_cache) {

        pr_err("fw_policy_init: creat policy cache err.\n");

        ret = -ENOMEM;
        goto err_out;
    }

    ret = 0;

err_out:

    return ret;
}

void fw_policy_cleanup(void)
{
    struct rb_node * next;
    struct fw_policy_node *data;
   
    write_lock_bh(&fw_policy_rwlock);
    /* free all objects before destroying caches */
    next = rb_first(&fw_policy_root);
    while (next) {
            data = rb_entry(next, struct fw_policy_node, node);
            next = rb_next(&data->node);
            rb_erase(&data->node, &fw_policy_root);
            kmem_cache_free (fw_policy_cache, data);
    }
    write_unlock_bh(&fw_policy_rwlock);
    kmem_cache_destroy (fw_policy_cache);
}

static struct fw_policy_node *
fw_policy_search(struct rb_root *root, __u32 ip)
{
    struct fw_policy_node *data;
  	struct rb_node *node = root->rb_node;

  	while (node) {
        data = rb_entry(node, struct fw_policy_node, node);

		if (ip < data->ip)
  			node = node->rb_left;
		else if (ip > data->ip)
  			node = node->rb_right;
		else
  			return data;
	}

	return NULL;
}

static int
fw_policy_insert(struct rb_root *root, struct fw_policy_node *data)
{
    struct fw_policy_node *this;
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	while (*new) {
        this = rb_entry(*new, struct fw_policy_node, node);

		parent = *new;
  		if (data->ip < this->ip)
  			new = &((*new)->rb_left);
  		else if (data->ip > this->ip)
  			new = &((*new)->rb_right);
  		else
  			return -1;
  	}
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 0;
}

void
fw_add_policy_node(__u32 ip)
{
    struct fw_policy_node *node;

    write_lock_bh(&fw_policy_rwlock);
    node = fw_policy_search(&fw_policy_root, ip);
    if (node != NULL){
        goto out;
    }

    node = kmem_cache_zalloc(fw_policy_cache, GFP_ATOMIC);
    if (node == NULL){
        goto out;
    }

    node->ip = ip;
    fw_policy_insert(&fw_policy_root, node);

out:
    write_unlock_bh(&fw_policy_rwlock);

    return;
}

void
fw_remove_policy_node(__u32 ip)
{
    struct fw_policy_node *node;

    write_lock_bh(&fw_policy_rwlock);
    node = fw_policy_search(&fw_policy_root, ip);
    if (node != NULL) {
        rb_erase(&node->node, &fw_policy_root);
    }
    write_unlock_bh(&fw_policy_rwlock);

    if (node != NULL) {
        kmem_cache_free(fw_policy_cache, node);
    }
}

int fw_check_ip_rule(__u32 ip)
{
    int res = 0;
    struct fw_policy_node *node;

    read_lock_bh(&fw_policy_rwlock);

    node = fw_policy_search(&fw_policy_root, ip);
    if (node != NULL){
        pr_info("fw_check_ip_rule match..0x%x\n", ip);
        res = -1;
    }

    read_unlock_bh(&fw_policy_rwlock);

    return res;
}

