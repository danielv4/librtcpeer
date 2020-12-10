#ifndef LIBRTCPEER_H
#define LIBRTCPEER_H

#ifdef __cplusplus

#endif

#ifdef __cplusplus
extern "C" {
#endif





#include "lfqueue.h"


// The C interface
typedef struct Connection RTCPeer;

RTCPeer* rtc_peer_new();

void del_rtc_peer(RTCPeer* obj);

int rtc_read_debug_message(RTCPeer* obj, uint8_t *bytes, int bytesLen);
	
int rtc_read_ice_candidate(RTCPeer* obj, uint8_t *bytes, int bytesLen);
	
int rtc_datachannel_read(RTCPeer* obj, uint8_t *bytes, int bytesLen);

int rtc_read_sdp(RTCPeer* obj, uint8_t *bytes, int bytesLen);

void rtc_init_base_channel(RTCPeer* obj);
	
void rtc_init_ssl(RTCPeer* obj);
	
void rtc_new_network_thread(RTCPeer* obj);

void rtc_new_worker_thread(RTCPeer* obj);

void rtc_new_signaling_thread(RTCPeer* obj);
	
void rtc_del_network(RTCPeer* obj);

void rtc_del_worker(RTCPeer* obj);
	
void rtc_del_signaling(RTCPeer* obj);
	
void rtc_del_ssl(RTCPeer* obj);
	
void rtc_new_factory_dependencies(RTCPeer* obj);
	
void rtc_set_ice_server(RTCPeer* obj, const char *stun_server);
	
void rtc_new_peer_connection(RTCPeer* obj);
	
void rtc_new_data_channel(RTCPeer* obj, const char *channel_name);
	
void rtc_new_sdp_offer(RTCPeer* obj);

void rtc_new_sdp_answer(RTCPeer* obj);
	
void rtc_parse_sdp_offer(RTCPeer* obj, const char *sdp);

void rtc_parse_sdp_answer(RTCPeer* obj, const char *sdp);
	
void rtc_add_ice_candidate(RTCPeer* obj, const char *ice);
	
void rtc_del_peer_connection(RTCPeer* obj);

void rtc_del_data_channel(RTCPeer* obj);

void rtc_del_peer_connection_factory(RTCPeer* obj);
	
int rtc_datachannel_get_state(RTCPeer* obj);

int rtc_datachannel_send_binary(RTCPeer* obj, char *buf, int buf_size);
	
int rtc_datachannel_send_text(RTCPeer* obj, char *buf, int buf_size);


#ifdef __cplusplus
}
#endif

#endif