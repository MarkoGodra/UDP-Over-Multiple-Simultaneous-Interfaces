#include "network.hpp"
#include <string.h>

// Ethernet header
void setup_ethernet_header(frame* frame_to_send, unsigned char*source_mac, unsigned char* dest_mac) {
    memcpy(&(frame_to_send->eh.src_address), source_mac, 6);
    memcpy(&(frame_to_send->eh.dest_address), dest_mac, 6);
    frame_to_send->eh.type = htons(0x0800);
}

// IP header
void setup_ip_header(frame* frame_to_send, unsigned char* src_ip, unsigned char* dest_ip) {
    frame_to_send->ih.version = 0x4;
    frame_to_send->ih.header_length = 0x5;
    frame_to_send->ih.tos = 0x00;
    frame_to_send->ih.length = htons(sizeof(frame) - sizeof(ethernet_header));
    frame_to_send->ih.identification = 0x0000;
    frame_to_send->ih.fragm_flags = 0x0000;
    frame_to_send->ih.fragm_offset = 0x0000;
    frame_to_send->ih.ttl = 0x7f;
    frame_to_send->ih.next_protocol = 0x11;

    unsigned int s = 0x5400 + (sizeof(frame) - sizeof(ethernet_header)) + 0x7F11;
    unsigned short u = (0xFFFF0000 & s) >> 16;
    s = (s & 0x0000FFFF) + u;

    frame_to_send->ih.checksum = htons(~(u_short)s);

	memcpy(&(frame_to_send->ih.src_addr), src_ip, 4);
	memcpy(&(frame_to_send->ih.dst_addr), dest_ip, 4);
    
/*
    memset(frame_to_send->ih.dst_addr, 0, 4);
    memset(frame_to_send->ih.src_addr, 0, 4);
    */
}

// UDP header
/* frameToSet - If 0 data frame will be set, if 1 ack frame will be set. */
void setup_udp_header(frame* frame_to_send, int frameToSet) {
    frame_to_send->uh.dest_port = htons(CLIENT_PORT);
    frame_to_send->uh.src_port = htons(SERVER_PORT);

    if(frameToSet) {
        frame_to_send->uh.datagram_length = htons(sizeof(udp_header)+ sizeof(unsigned long long));
    }
    else {
        frame_to_send->uh.datagram_length = htons(sizeof(udp_header)+ sizeof(fc_header));
    }

    frame_to_send->uh.checksum = htons(0);
}

// Frame count header
void setup_fc_header(frame* frame_to_send, unsigned long long frame_cnt, unsigned long long total_num_of_frames, char* buff, unsigned int data_len) {
    frame_to_send->fch.frame_count = frame_cnt;
    frame_to_send->fch.num_of_total_frames = total_num_of_frames;
    frame_to_send->fch.data_len = data_len;

    for(int i = 0; i < data_len; i++) {
        frame_to_send->fch.data[i] = buff[i];
    }
    //memcpy(frame_to_send->fch.data, buff, data_len);
}

void fill_data_frame(frame* frame_to_send, unsigned char* source_mac, unsigned char* dest_mac, char* buff, unsigned long long frame_cnt, unsigned long long total_num_of_frames, unsigned int data_len, unsigned char* src_ip, unsigned char* dest_ip) {
    setup_ethernet_header(frame_to_send, source_mac, dest_mac);

    setup_ip_header(frame_to_send, src_ip, dest_ip);

    setup_udp_header(frame_to_send, 0);

    setup_fc_header(frame_to_send, frame_cnt, total_num_of_frames, buff, data_len);
}

void fill_ack_frame(ack_frame* frame_to_send, unsigned char* source_mac, unsigned char* dest_mac, unsigned long long ack_number, unsigned char* src_ip, unsigned char* dest_ip) {
    setup_ethernet_header((frame*)frame_to_send, source_mac, dest_mac);

    setup_ip_header((frame*)frame_to_send, src_ip, dest_ip);

    setup_udp_header((frame*)frame_to_send, 1);

    frame_to_send->ack_num = ack_number;
}

pcap_if_t* select_device(pcap_if_t* devices) {
    int i = 0;
    pcap_if_t* device;
    int device_number;

    // Count devices and provide jumping to the selected device
    // Print the list
    for(device=devices; device; device=device->next){
        printf("%d. %s", ++i, device->name);
        if (device->description)
            printf(" (%s)\n", device->description);
        else
            printf(" (No description available)\n");
    }

    // Check if list is empty
    if (i==0){
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter the output interface number (1-%d):",i);
    scanf("%d", &device_number);

    if(device_number < 1 || device_number > i){
        printf("\nInterface number out of range.\n");
        exit(EXIT_FAILURE);
    }

    // Select the first device...
    device=devices;
    // ...and then jump to chosen devices
    for (i=0; i<device_number-1; i++){
        device=device->next;
    }

    return device;
}

