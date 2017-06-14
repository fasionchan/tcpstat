/**
 * FileName:   tcpstat.c
 * Author:     Fasion Chan
 * @contact:   fasionchan@gmail.com
 * @version:   $Id$
 *
 * Description:
 *
 * Changelog:
 *
 **/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <net/tcp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fasion Chan");
MODULE_DESCRIPTION("TCP state statistics");
MODULE_VERSION("1.0");

// 状态名列表
static char *state_names[] = {
    NULL,
    "ESTABLISHED",
    "SYN_SENT",
    "SYN_RECV",
    "FIN_WAIT1",
    "FIN_WAIT2",
    "TIME_WAIT",
    "CLOSE",
    "CLOSE_WAIT",
    "LAST_ACK",
    "LISTEN",
    "CLOSING",
    NULL
};


static void stat_sock_list(struct hlist_nulls_head *head, spinlock_t *lock,
    unsigned int state_counters[])
{
    // 套接字节点指针(用于遍历)
    struct sock *sk;
    struct hlist_nulls_node *node;

    // 链表为空直接返回
    if (hlist_nulls_empty(head)) {
        return;
    }

    // 自旋锁锁定
    spin_lock_bh(lock);

    // 遍历套接字链表
    sk = sk_nulls_head(head);
    sk_nulls_for_each_from(sk, node) {
        if (sk->sk_state < TCP_MAX_STATES) {
            // 自增状态计数器
            state_counters[sk->sk_state]++;
        }
    }

    // 自旋锁解锁
    spin_unlock_bh(lock);
}


static int tcpstat_seq_show(struct seq_file *seq, void *v)
{
    // 状态计数器
    unsigned int state_counters[TCP_MAX_STATES] = { 0 };
    unsigned int state;

    // TCP套接字哈希槽序号
    unsigned int bucket;

    // 先遍历Listen状态
    for (bucket = 0; bucket < INET_LHTABLE_SIZE; bucket++) {
        struct inet_listen_hashbucket *ilb;

        // 哈希槽
        ilb = &tcp_hashinfo.listening_hash[bucket];

        // 遍历链表并统计
        stat_sock_list(&ilb->head, &ilb->lock, state_counters);
    }

    // 遍历其他状态
    for (bucket = 0; bucket < tcp_hashinfo.ehash_mask; bucket++) {
        struct inet_ehash_bucket *ilb;
        spinlock_t *lock;

        // 哈希槽链表
        ilb = &tcp_hashinfo.ehash[bucket];
        // 保护锁
        lock = inet_ehash_lockp(&tcp_hashinfo, bucket);

        // 遍历链表并统计
        stat_sock_list(&ilb->chain, lock, state_counters);
    }

    // 遍历状态输出统计值
    for (state = TCP_ESTABLISHED; state < TCP_MAX_STATES; state++) {
        seq_printf(seq, "%-12s: %d\n", state_names[state], state_counters[state]);
    }

    return 0;
}


static int tcpstat_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, tcpstat_seq_show, NULL);
}


static const struct file_operations tcpstat_file_ops = {
    .owner   = THIS_MODULE,
    .open    = tcpstat_seq_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release
};


static __init int tcpstat_init(void)
{
    proc_create("tcpstat", 0, NULL, &tcpstat_file_ops);
    return 0;
}


static __exit void tcpstat_exit(void)
{
    remove_proc_entry("tcpstat", NULL);
}

module_init(tcpstat_init);
module_exit(tcpstat_exit);
