#ifndef _XEMAC_DMA_H_
#define _XEMAC_DMA_H_

#include "xemac_adapter.h"

XStatus hw_intf_init_dma(struct xemac_adapter_context *adapter_context);

void emacps_dma_send_handler(void *arg);
void emacps_dma_recv_handler(void *arg);
void emacps_dma_error_handler(void *arg);

#endif /* _XEMAC_DMA_H_ */
