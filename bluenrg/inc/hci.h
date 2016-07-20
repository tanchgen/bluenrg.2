/*
 * hci.h
 *
 *  Created on: Jul 18, 2016
 *      Author: jet
 */

#ifndef HCI_H_
#define HCI_H_

/* HCI library functions. */
void hci_init(void);

int hci_send_req(struct hci_request *r, uint8_t async);



#endif /* HCI_H_ */
