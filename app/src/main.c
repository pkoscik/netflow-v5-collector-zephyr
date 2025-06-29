/*
 * Copyright (c) 2025 Patryk Kościk
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(netflow_listener, LOG_LEVEL_DBG);

#define NETFLOW_V5_PORT    2055
#define PACKET_HEADER_SIZE 24
#define FLOW_RECORD_SIZE   48
#define RECV_BUFFER_SIZE   (64 * 1024)

struct netflow_v5_header {
    uint16_t version;
    uint16_t count;
    uint32_t sys_uptime;
    uint32_t unix_secs;
    uint32_t unix_nsecs;
    uint32_t flow_sequence;
    uint8_t engine_type;
    uint8_t engine_id;
    uint16_t sampling_interval;
} __packed;

struct netflow_v5_record {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t next_hop;
    uint16_t input;
    uint16_t output;
    uint32_t d_pkts;
    uint32_t d_octets;
    uint32_t first;
    uint32_t last;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t pad1;
    uint8_t tcp_flags;
    uint8_t protocol;
    uint8_t tos;
    uint16_t src_as;
    uint16_t dst_as;
    uint8_t src_mask;
    uint8_t dst_mask;
    uint16_t pad2;
} __packed;

void netflow_listener_thread(void)
{
    int sock;
    struct sockaddr_in addr;
    char recv_buf[RECV_BUFFER_SIZE] __aligned(4);
    socklen_t addr_len = sizeof(addr);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0) {
        LOG_ERR("Failed to create socket: %d", errno);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NETFLOW_V5_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERR("Failed to bind socket: %d", errno);
        close(sock);
        return;
    }

    LOG_INF("Listening on UDP port %d", NETFLOW_V5_PORT);

    while(1) {
        ssize_t received = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr, &addr_len);

        if(received < (ssize_t)sizeof(struct netflow_v5_header)) {
            LOG_WRN("Packet too short for NetFlow v5 header");
            continue;
        }

        struct netflow_v5_header *hdr = (struct netflow_v5_header *)recv_buf;
        uint16_t version = ntohs(hdr->version);
        uint16_t flow_count = ntohs(hdr->count);

        if(version != 5) {
            LOG_WRN("Non-NetFlow v5 packet received (version %u) - skipping", version);
            continue;
        }

        char sender_ip[NET_IPV4_ADDR_LEN];
        net_addr_ntop(AF_INET, &addr.sin_addr, sender_ip, sizeof(sender_ip));

        LOG_INF("NetFlow v5 packet: %u flows from %s", flow_count, sender_ip);
        LOG_DBG("SysUptime=%u, UnixSecs=%u, FlowSeq=%u", ntohl(hdr->sys_uptime), ntohl(hdr->unix_secs),
                ntohl(hdr->flow_sequence));

        size_t expected_size = sizeof(struct netflow_v5_header) + flow_count * sizeof(struct netflow_v5_record);
        if(received < (ssize_t)expected_size) {
            LOG_WRN("Packet truncated: expected %u flow records", flow_count);
            continue;
        }

        struct netflow_v5_record *records = (struct netflow_v5_record *)(recv_buf + sizeof(struct netflow_v5_header));
        for(int i = 0; i < flow_count; ++i) {
            struct netflow_v5_record *rec = &records[i];

            char src_ip[NET_IPV4_ADDR_LEN];
            char dst_ip[NET_IPV4_ADDR_LEN];
            char nh_ip[NET_IPV4_ADDR_LEN];

            net_addr_ntop(AF_INET, &rec->src_addr, src_ip, sizeof(src_ip));
            net_addr_ntop(AF_INET, &rec->dst_addr, dst_ip, sizeof(dst_ip));
            net_addr_ntop(AF_INET, &rec->next_hop, nh_ip, sizeof(nh_ip));

            LOG_INF("Flow %d: %s:%u → %s:%u proto=%u bytes=%u packets=%u", i, src_ip, ntohs(rec->src_port), dst_ip,
                    ntohs(rec->dst_port), rec->protocol, ntohl(rec->d_octets), ntohl(rec->d_pkts));
        }
    }

    //  unreachable
    close(sock);
}

K_THREAD_DEFINE(netflow_listener_tid, 2048 + RECV_BUFFER_SIZE, netflow_listener_thread, NULL, NULL, NULL, 7, 0, 0);
