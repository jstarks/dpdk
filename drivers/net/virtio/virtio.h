/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 * Copyright(c) 2021 Red Hat, Inc.
 */

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include <rte_ether.h>

/* The feature bitmap for virtio net */
#define VIRTIO_NET_F_CSUM	0	/* Host handles pkts w/ partial csum */
#define VIRTIO_NET_F_GUEST_CSUM	1	/* Guest handles pkts w/ partial csum */
#define VIRTIO_NET_F_MTU	3	/* Initial MTU advice. */
#define VIRTIO_NET_F_MAC	5	/* Host has given MAC address. */
#define VIRTIO_NET_F_GUEST_TSO4	7	/* Guest can handle TSOv4 in. */
#define VIRTIO_NET_F_GUEST_TSO6	8	/* Guest can handle TSOv6 in. */
#define VIRTIO_NET_F_GUEST_ECN	9	/* Guest can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_GUEST_UFO	10	/* Guest can handle UFO in. */
#define VIRTIO_NET_F_HOST_TSO4	11	/* Host can handle TSOv4 in. */
#define VIRTIO_NET_F_HOST_TSO6	12	/* Host can handle TSOv6 in. */
#define VIRTIO_NET_F_HOST_ECN	13	/* Host can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_HOST_UFO	14	/* Host can handle UFO in. */
#define VIRTIO_NET_F_MRG_RXBUF	15	/* Host can merge receive buffers. */
#define VIRTIO_NET_F_STATUS	16	/* virtio_net_config.status available */
#define VIRTIO_NET_F_CTRL_VQ	17	/* Control channel available */
#define VIRTIO_NET_F_CTRL_RX	18	/* Control channel RX mode support */
#define VIRTIO_NET_F_CTRL_VLAN	19	/* Control channel VLAN filtering */
#define VIRTIO_NET_F_CTRL_RX_EXTRA 20	/* Extra RX mode control support */
#define VIRTIO_NET_F_GUEST_ANNOUNCE 21	/* Guest can announce device on the network */
#define VIRTIO_NET_F_MQ		22	/* Device supports Receive Flow Steering */
#define VIRTIO_NET_F_CTRL_MAC_ADDR 23	/* Set MAC address */

/*
 * Do we get callbacks when the ring is completely used,
 * even if we've suppressed them?
 */
#define VIRTIO_F_NOTIFY_ON_EMPTY	24

/* Can the device handle any descriptor layout? */
#define VIRTIO_F_ANY_LAYOUT		27

/* We support indirect buffer descriptors */
#define VIRTIO_RING_F_INDIRECT_DESC	28

#define VIRTIO_F_VERSION_1		32
#define VIRTIO_F_IOMMU_PLATFORM	33
#define VIRTIO_F_RING_PACKED		34

/*
 * Some VirtIO feature bits (currently bits 28 through 31) are
 * reserved for the transport being used (eg. virtio_ring), the
 * rest are per-device feature bits.
 */
#define VIRTIO_TRANSPORT_F_START 28
#define VIRTIO_TRANSPORT_F_END   34

/*
 * Inorder feature indicates that all buffers are used by the device
 * in the same order in which they have been made available.
 */
#define VIRTIO_F_IN_ORDER 35

/*
 * This feature indicates that memory accesses by the driver and the device
 * are ordered in a way described by the platform.
 */
#define VIRTIO_F_ORDER_PLATFORM 36

/*
 * This feature indicates that the driver passes extra data (besides
 * identifying the virtqueue) in its device notifications.
 */
#define VIRTIO_F_NOTIFICATION_DATA 38

/* Device set linkspeed and duplex */
#define VIRTIO_NET_F_SPEED_DUPLEX 63

/*
 * The Guest publishes the used index for which it expects an interrupt
 * at the end of the avail ring. Host should ignore the avail->flags field
 *
 * The Host publishes the avail index for which it expects a kick
 * at the end of the used ring. Guest should ignore the used->flags field.
 */
#define VIRTIO_RING_F_EVENT_IDX		29

#define VIRTIO_NET_S_LINK_UP	1	/* Link is up */
#define VIRTIO_NET_S_ANNOUNCE	2	/* Announcement is needed */

struct virtio_hw {
	struct virtqueue **vqs;
	uint64_t guest_features;
	uint16_t vtnet_hdr_size;
	uint8_t started;
	uint8_t weak_barriers;
	uint8_t vlan_strip;
	uint8_t has_tx_offload;
	uint8_t has_rx_offload;
	uint8_t use_vec_rx;
	uint8_t use_vec_tx;
	uint8_t use_inorder_rx;
	uint8_t use_inorder_tx;
	uint8_t opened;
	uint16_t port_id;
	uint8_t mac_addr[RTE_ETHER_ADDR_LEN];
	uint32_t speed;  /* link speed in MB */
	uint8_t duplex;
	uint8_t use_msix;
	uint16_t max_mtu;
	/*
	 * App management thread and virtio interrupt handler thread
	 * both can change device state, this lock is meant to avoid
	 * such a contention.
	 */
	rte_spinlock_t state_lock;
	struct rte_mbuf **inject_pkts;
	uint16_t max_queue_pairs;
	uint64_t req_guest_features;
	struct virtnet_ctl *cvq;
};

struct virtio_ops {
	void (*read_dev_cfg)(struct virtio_hw *hw, size_t offset, void *dst, int len);
	void (*write_dev_cfg)(struct virtio_hw *hw, size_t offset, const void *src, int len);
	uint8_t (*get_status)(struct virtio_hw *hw);
	void (*set_status)(struct virtio_hw *hw, uint8_t status);
	uint64_t (*get_features)(struct virtio_hw *hw);
	void (*set_features)(struct virtio_hw *hw, uint64_t features);
	int (*features_ok)(struct virtio_hw *hw);
	uint8_t (*get_isr)(struct virtio_hw *hw);
	uint16_t (*set_config_irq)(struct virtio_hw *hw, uint16_t vec);
	uint16_t (*set_queue_irq)(struct virtio_hw *hw, struct virtqueue *vq, uint16_t vec);
	uint16_t (*get_queue_num)(struct virtio_hw *hw, uint16_t queue_id);
	int (*setup_queue)(struct virtio_hw *hw, struct virtqueue *vq);
	void (*del_queue)(struct virtio_hw *hw, struct virtqueue *vq);
	void (*notify_queue)(struct virtio_hw *hw, struct virtqueue *vq);
	void (*intr_detect)(struct virtio_hw *hw);
	int (*dev_close)(struct virtio_hw *hw);
};

/*
 * This structure stores per-process data. Only virtio_ops for now.
 */
struct virtio_hw_internal {
	const struct virtio_ops *virtio_ops;
};

#define VIRTIO_OPS(hw)	(virtio_hw_internal[(hw)->port_id].virtio_ops)

extern struct virtio_hw_internal virtio_hw_internal[RTE_MAX_ETHPORTS];


static inline int
virtio_with_feature(struct virtio_hw *hw, uint64_t bit)
{
	return (hw->guest_features & (1ULL << bit)) != 0;
}

static inline int
virtio_with_packed_queue(struct virtio_hw *hw)
{
	return virtio_with_feature(hw, VIRTIO_F_RING_PACKED);
}

uint64_t virtio_negotiate_features(struct virtio_hw *hw, uint64_t host_features);

#endif /* _VIRTIO_H_ */