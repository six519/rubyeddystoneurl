/*
*
*
* Thanks to: https://github.com/noble/noble
*
*/
#include "ruby.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

VALUE RubyEddystoneURL = Qnil;
void Init_rubyeddystoneurl();
VALUE method_scan(VALUE, VALUE);

void Init_rubyeddystoneurl(){
    VALUE RubyEddystoneURL = rb_define_module("RubyEddystoneURL");
    rb_define_method(RubyEddystoneURL, "scan", method_scan,1);
}

VALUE method_scan(VALUE self, VALUE timeout) {
    VALUE retArray = rb_ary_new();

    int hciDeviceId = 0;
    int hciSocket;
    struct hci_dev_info hciDevInfo;

    struct hci_filter oldHciFilter;
    struct hci_filter newHciFilter;
    socklen_t oldHciFilterLen;

    int currentAdapterState;
    int timeoutCounter = 0;

    unsigned char hciEventBuf[HCI_MAX_EVENT_SIZE];
    int hciEventLen;
    evt_le_meta_event *leMetaEvent;
    le_advertising_info *leAdvertisingInfo;
    char btAddress[18];
    int i;
    int8_t rssi;
    fd_set rfds;
    struct timeval tv;
    int selectRetval;
    char btInfo[500];

    if(TYPE(timeout) != T_FIXNUM) {
        rb_raise(rb_eTypeError, "Invalid parameter");
        return Qnil;
    }

    memset(&hciDevInfo, 0x00, sizeof(hciDevInfo));

    //use first available device
    hciDeviceId = hci_get_route(NULL);

    if (hciDeviceId < 0) {
        hciDeviceId = 0; // use device 0, if device id is invalid
    }

    // setup HCI socket
    hciSocket = hci_open_dev(hciDeviceId);

    if (hciSocket == -1) {
        rb_raise(rb_eStandardError, "Unsupported");
        return Qnil;
    }
    hciDevInfo.dev_id = hciDeviceId;

    // get old HCI filter
    oldHciFilterLen = sizeof(oldHciFilter);
    getsockopt(hciSocket, SOL_HCI, HCI_FILTER, &oldHciFilter, &oldHciFilterLen);

    // setup new HCI filter
    hci_filter_clear(&newHciFilter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &newHciFilter);
    hci_filter_set_event(EVT_LE_META_EVENT, &newHciFilter);
    setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &newHciFilter, sizeof(newHciFilter));

    // disable scanning, it may have been left on, if so hci_le_set_scan_parameters will fail without this
    hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);

    // get HCI dev info for adapter state
    ioctl(hciSocket, HCIGETDEVINFO, (void *)&hciDevInfo);
    currentAdapterState = hci_test_bit(HCI_UP, &hciDevInfo.flags);

    if (!currentAdapterState) {
        //powered off
        currentAdapterState = 0;
    } else if (hci_le_set_scan_parameters(hciSocket, 0x01, htobs(0x0010), htobs(0x0010), 0x00, 0, 1000) < 0) {
        if (EPERM == errno) {
            //unauthorized
            currentAdapterState = 1;
        } else if (EIO == errno) {
            //unsupported
            currentAdapterState = 2;
        } else {
            //unknown
            currentAdapterState = 3;
        }
    } else {
        //powered on
        currentAdapterState = 4;
    }

    if(currentAdapterState == 4) {
        //scan devices
        hci_le_set_scan_enable(hciSocket, 0x00, 1, 1000);
        hci_le_set_scan_enable(hciSocket, 0x01, 1, 1000);

        while(1) {
            FD_ZERO(&rfds);
            FD_SET(hciSocket, &rfds);

            tv.tv_sec = 1;
            tv.tv_usec = 0;

            selectRetval = select(hciSocket + 1, &rfds, NULL, NULL, &tv);

            if(selectRetval) {
                //get scan devices
                hciEventLen = read(hciSocket, hciEventBuf, sizeof(hciEventBuf));
                leMetaEvent = (evt_le_meta_event *)(hciEventBuf + (1 + HCI_EVENT_HDR_SIZE));
                hciEventLen -= (1 + HCI_EVENT_HDR_SIZE);

                if (leMetaEvent->subevent == 0x02) {
                    leAdvertisingInfo = (le_advertising_info *)(leMetaEvent->data + 1);
                    ba2str(&leAdvertisingInfo->bdaddr, btAddress);

                    VALUE hash = rb_hash_new();
                    rb_hash_aset(hash, rb_str_new2("address"), rb_str_new2(btAddress));

                    rssi = *(leAdvertisingInfo->data + leAdvertisingInfo->length);
                    
                    rb_hash_aset(hash, rb_str_new2("rssi"), rb_int_new((int)rssi));
                    
                    btInfo[0] = '\0';

                    for (i = 0; i < leAdvertisingInfo->length; i++) {
                        sprintf(btInfo + strlen(btInfo), "%02x", leAdvertisingInfo->data[i]);
                    }

                    //printf("%s\n", btInfo);

                    rb_hash_aset(hash, rb_str_new2("info"), rb_str_new2(btInfo));
                    rb_ary_push(retArray, hash);
                }
            }

            timeoutCounter += 1;
            sleep(1);
            
            if(timeoutCounter == FIX2INT(timeout)) {
                //stop scan
                hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);
                break;
            }

        }
    }

    // restore original filter
    setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &oldHciFilter, sizeof(oldHciFilter));
    // disable LE scan
    hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);
    close(hciSocket);

    return retArray;
}
