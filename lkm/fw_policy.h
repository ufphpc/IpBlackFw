#ifndef _FW_POLICY_H
#define _FW_POLICY_H

int  fw_policy_init(void);
void fw_policy_cleanup(void);
int  fw_check_ip_rule(__u32 ip);
void fw_add_policy_node(__u32 ip);
void fw_remove_policy_node(__u32 ip);

#endif