#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>








// google webrtc headers
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <rtc_base/physical_socket_server.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/thread.h>
#include <system_wrappers/include/field_trial.h>
#include "lfqueue.h"
#include <json/json.h>
#include "librtcpeer.h"


class Connection {
	public:
	
		std::unique_ptr<rtc::Thread> network_thread;
		std::unique_ptr<rtc::Thread> worker_thread;
		std::unique_ptr<rtc::Thread> signaling_thread;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
		webrtc::PeerConnectionInterface::RTCConfiguration configuration;	
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;
		rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;
		std::string sdp_type;
		
		lfqueue_t *queue_debug;
		lfqueue_t *queue_ice;
		lfqueue_t *queue_sdp;
		lfqueue_t *queue_data;
		
		void rtc_init_base_channel() {
			
			// new queue
			queue_debug = (lfqueue_t *)malloc(sizeof(lfqueue_t));
			if (lfqueue_init(queue_debug) == -1) {

			}
			
			// new queue
			queue_ice = (lfqueue_t *)malloc(sizeof(lfqueue_t));
			if (lfqueue_init(queue_ice) == -1) {

			}
			
			// new queue
			queue_data = (lfqueue_t *)malloc(sizeof(lfqueue_t));
			if (lfqueue_init(queue_data) == -1) {

			}
			
			// new queue
			queue_sdp = (lfqueue_t *)malloc(sizeof(lfqueue_t));
			if (lfqueue_init(queue_sdp) == -1) {

			}
		}	

		int rtc_read_debug_message(uint8_t *bytes, int bytesLen) {

			int val = 0;
			void *ptr = lfqueue_deq(queue_debug);

			if(ptr != NULL) {
				
				memcpy(bytes, (uint8_t *)ptr, bytesLen);
				val = bytesLen;
			}
			
			return val;
		}
		
		int rtc_read_sdp(uint8_t *bytes, int bytesLen) {

			int val = 0;
			void *ptr = lfqueue_deq(queue_sdp);

			if(ptr != NULL) {
				
				memcpy(bytes, (uint8_t *)ptr, bytesLen);
				val = bytesLen;
			}
			
			return val;
		}
		
		int rtc_read_ice_candidate(uint8_t *bytes, int bytesLen) {

			int val = 0;
			void *ptr = lfqueue_deq(queue_ice);

			if(ptr != NULL) {
				
				memcpy(bytes, (uint8_t *)ptr, bytesLen);
				val = bytesLen;
			}
			
			return val;
		}
		
		int rtc_datachannel_read(uint8_t *bytes, int bytesLen) {
			
			int val = 0;
			void *ptr = lfqueue_deq(queue_data);

			if(ptr != NULL) {
				
				memcpy(bytes, (uint8_t *)ptr, bytesLen);
				val = bytesLen;
			}
			
			return val;
		}		
		
		void rtc_init_ssl() {
			
			webrtc::field_trial::InitFieldTrialsFromString("");
	
			// init ssl
			rtc::InitializeSSL();	
		}
		
		void rtc_new_network_thread() {
			
			network_thread = rtc::Thread::CreateWithSocketServer();
			network_thread->Start();		
		}
		
		void rtc_new_worker_thread() {
			
			worker_thread = rtc::Thread::Create();
			worker_thread->Start();		
		}
		
		void rtc_new_signaling_thread() {
			
			signaling_thread = rtc::Thread::Create();
			signaling_thread->Start();
		}
		
		void rtc_del_network() {
			
			network_thread->Stop();
		}
		
		void rtc_del_worker() {
			
			worker_thread->Stop();
		}
		
		void rtc_del_signaling() {
			
			signaling_thread->Stop();
		}
		
		void rtc_del_ssl() {
			
			rtc::CleanupSSL();
		}
		
		void rtc_new_factory_dependencies() {
			
			webrtc::PeerConnectionFactoryDependencies dependencies;
			dependencies.network_thread   = network_thread.get();
			dependencies.worker_thread    = worker_thread.get();
			dependencies.signaling_thread = signaling_thread.get();
			peer_connection_factory       = webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));

			if (peer_connection_factory.get() == nullptr) {
				
				printf("Error on CreateModularPeerConnectionFactory. \n");
			}			
		}
		
		void rtc_set_ice_server(const char *stun_server_s) {
			
			// name
			std::string stun_server;
			stun_server.assign(stun_server_s, strlen(stun_server_s));

			webrtc::PeerConnectionInterface::IceServer ice_server;
			ice_server.uri = stun_server;
			configuration.servers.push_back(ice_server);
		}
		
		void rtc_new_peer_connection() {
			
			peer_connection = peer_connection_factory->CreatePeerConnection(configuration, nullptr, nullptr, &pco);
		}
		
		void rtc_new_data_channel(const char *channel_name_s) {

			// name
			std::string channel_name;
			channel_name.assign(channel_name_s, strlen(channel_name_s));

			webrtc::DataChannelInit config;
			data_channel = peer_connection->CreateDataChannel(channel_name, &config);
			data_channel->RegisterObserver(&dco);	
		}
		
		void rtc_new_sdp_offer() {

			peer_connection->CreateOffer(csdo, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
		}
		
		
		void rtc_new_sdp_answer() {

			peer_connection->CreateAnswer(csdo, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
		}
		
		void rtc_parse_sdp_offer(const char *sdp) {

			// sdp
			std::string sdp_str;
			sdp_str.assign(sdp, strlen(sdp));
			
			// parse answer
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface *session_description(webrtc::CreateSessionDescription("offer", sdp_str, &error));
			if (session_description == nullptr) {
				printf("check error \n");
			}
			
			peer_connection->SetRemoteDescription(ssdo, session_description);			
		}


		void rtc_parse_sdp_answer(const char *sdp) {

			// sdp
			std::string sdp_str;
			sdp_str.assign(sdp, strlen(sdp));
			
			// parse answer
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface *session_description(webrtc::CreateSessionDescription("answer", sdp_str, &error));
			if (session_description == nullptr) {
				printf("check error \n");
			}
			
			peer_connection->SetRemoteDescription(ssdo, session_description);			
		}
		
		void rtc_add_ice_candidate(const char *ice_s) {

			// ice
			std::string ice_str;
			ice_str.assign(ice_s, strlen(ice_s));
			
			// add remote Candidate
			webrtc::SdpParseError err_sdp;
			webrtc::IceCandidateInterface *ice = CreateIceCandidate("data", 0, ice_str, &err_sdp);
			if (!err_sdp.line.empty() && !err_sdp.description.empty()) {
				printf("check error \n");
			}
			
			peer_connection->AddIceCandidate(ice);
		}
		
		void rtc_del_peer_connection() {

			peer_connection->Close();
			peer_connection = nullptr;
		}	

		void rtc_del_data_channel() {

			data_channel = nullptr;
		}	

		void rtc_del_peer_connection_factory() {

			peer_connection_factory = nullptr;
		}			
		
		int rtc_datachannel_get_state() {

			return data_channel->state();
		}	

		int rtc_datachannel_send_binary(char *buf, int buf_size) {

			int stat = 0;
			webrtc::DataBuffer buffer(rtc::CopyOnWriteBuffer(buf, buf_size), true);
			bool check = data_channel->Send(buffer);
			
			if(check == true) {
				stat = 1;
			}
			
			return stat;
		}


		int rtc_datachannel_send_text(char *buf, int buf_size) {
			
			int stat = 0;
			webrtc::DataBuffer buffer(rtc::CopyOnWriteBuffer(buf, buf_size), false);
			bool check = data_channel->Send(buffer);
			
			if(check == true) {
				stat = 1;
			}
			
			return stat;
		}		


		void onSuccessCSD(webrtc::SessionDescriptionInterface *desc) {
			
			// get
			peer_connection->SetLocalDescription(ssdo, desc);
			std::string sdp;
			desc->ToString(&sdp);
			
			// bytes
			uint8_t *buf = (uint8_t *)malloc(65000);
			memcpy(buf, sdp.c_str(), strlen(sdp.c_str()));
			lfqueue_enq(queue_sdp, (void *)buf);
		}

		void onIceCandidate(const webrtc::IceCandidateInterface *candidate) {

			// get
			std::string candidate_str;
			candidate->ToString(&candidate_str);

			// bytes
			uint8_t *buf = (uint8_t *)malloc(65000);
			memcpy(buf, candidate_str.c_str(), strlen(candidate_str.c_str()));
			lfqueue_enq(queue_ice, (void *)buf);
		}

	class PCO : public webrtc::PeerConnectionObserver {
		
		private:
			Connection &parent;

		public:
			PCO(Connection &parent) : parent(parent) {
				
			}

			void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {
				
				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_signaling_change"] = new_state;
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);				
			};

			void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_add_stream"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);	
			};

			void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_remove_stream"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);	
			};

			void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
				
				// set new data channel
				parent.data_channel = data_channel;
				parent.data_channel->RegisterObserver(&parent.dco);

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_data_channel"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);	
			};

			void OnRenegotiationNeeded() override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_renegotiation_needed"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};

			void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_ice_connection_change"] = new_state;
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};

			void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_ice_gathering_change"] = new_state;
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};

			void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override {
				
				parent.onIceCandidate(candidate);
			};
	};

	class DCO : public webrtc::DataChannelObserver {
		private:
			Connection &parent;

		public:
			DCO(Connection &parent) : parent(parent) {
			}

			void OnStateChange() override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_state_change"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};

			void OnMessage(const webrtc::DataBuffer &buffer) override {
				
				// message
				std::string string_g = std::string(buffer.data.data<char>(), buffer.data.size());
				int webrtc_buffer_size = buffer.data.size();
				const char *buf_g = string_g.c_str();
				

				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, buf_g, webrtc_buffer_size);
				lfqueue_enq(parent.queue_data, (void *)buf);						
			};

			void OnBufferedAmountChange(uint64_t previous_amount) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_buffered_amount_change"] = previous_amount;
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);	
			};
	};

	class CSDO : public webrtc::CreateSessionDescriptionObserver {
		private:
			Connection &parent;

		public:
			CSDO(Connection &parent) : parent(parent) {
			}

			void OnSuccess(webrtc::SessionDescriptionInterface *desc) override {
				parent.onSuccessCSD(desc);
			};

			void OnFailure(const std::string &error) override {
				
				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_failure"] = error;
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};
	};

	class SSDO : public webrtc::SetSessionDescriptionObserver {
		private:
			Connection &parent;

		public:
			SSDO(Connection &parent) : parent(parent) {
			}

			void OnSuccess() override {
				
				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_success_set"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};

			void OnFailure(const std::string &error) override {

				// json
				Json::Value val;
				Json::StreamWriterBuilder builder;
				builder["commentStyle"] = "None";
				builder["indentation"] = "";
				val["on_failure_set"] = "event";
				const std::string output = Json::writeString(builder, val);
				
				// bytes
				uint8_t *buf = (uint8_t *)malloc(65000);
				memcpy(buf, output.c_str(), strlen(output.c_str()));
				lfqueue_enq(parent.queue_debug, (void *)buf);
			};
	};

	PCO pco;
	DCO dco;
	rtc::scoped_refptr<CSDO> csdo;
	rtc::scoped_refptr<SSDO> ssdo;

	Connection() :
		pco(*this),
		dco(*this),
		csdo(new rtc::RefCountedObject<CSDO>(*this)),
		ssdo(new rtc::RefCountedObject<SSDO>(*this)) {
	}
};







extern "C" {
	
	Connection* rtc_peer_new() {
		return new Connection;
	}
	
	void del_rtc_peer(Connection* obj) {
		delete obj;
	}
	
	int rtc_read_debug_message(Connection* obj, uint8_t *bytes, int bytesLen) {

		int val = obj->rtc_read_debug_message(bytes, bytesLen);
		return val;
	}
	
	int rtc_read_ice_candidate(Connection* obj, uint8_t *bytes, int bytesLen) {

		int val = obj->rtc_read_ice_candidate(bytes, bytesLen);
		return val;
	}
	
	int rtc_read_sdp(Connection* obj, uint8_t *bytes, int bytesLen) {

		int val = obj->rtc_read_sdp(bytes, bytesLen);
		return val;
	}
	
	int rtc_datachannel_read(Connection* obj, uint8_t *bytes, int bytesLen) {
		
		int val = obj->rtc_datachannel_read(bytes, bytesLen);
		return val;
	}	
	
	void rtc_init_base_channel(Connection* obj) {
	
		obj->rtc_init_base_channel();
	}
	
	void rtc_init_ssl(Connection* obj) {
		
		obj->rtc_init_ssl();	
	}
	
	void rtc_new_network_thread(Connection* obj) {
		
		obj->rtc_new_network_thread();	
	}
		
	void rtc_new_worker_thread(Connection* obj) {
		
		obj->rtc_new_worker_thread();	
	}
		
	void rtc_new_signaling_thread(Connection* obj) {
		
		obj->rtc_new_signaling_thread();	
	}
	
	void rtc_del_network(Connection* obj) {
		
		obj->rtc_del_network();
	}
		
	void rtc_del_worker(Connection* obj) {
		
		obj->rtc_del_worker();
	}
	
	void rtc_del_signaling(Connection* obj) {
		
		obj->rtc_del_signaling();
	}
	
	void rtc_del_ssl(Connection* obj) {
		
		obj->rtc_del_ssl();
	}
	
	void rtc_new_factory_dependencies(Connection* obj) {
		
		obj->rtc_new_factory_dependencies();
	}
	
	void rtc_set_ice_server(Connection* obj, const char *stun_server) {
		
		obj->rtc_set_ice_server(stun_server);
	}
	
	void rtc_new_peer_connection(Connection* obj) {
		
		obj->rtc_new_peer_connection();
	}
	
	void rtc_new_data_channel(Connection* obj, const char *channel_name) {
		
		obj->rtc_new_data_channel(channel_name);
	}
	
	void rtc_new_sdp_offer(Connection* obj) {
		
		obj->rtc_new_sdp_offer();
	}
	
	void rtc_new_sdp_answer(Connection* obj) {

		obj->rtc_new_sdp_answer();
	}
	
	void rtc_parse_sdp_offer(Connection* obj, const char *sdp) {

		obj->rtc_parse_sdp_offer(sdp);
	}

	void rtc_parse_sdp_answer(Connection* obj, const char *sdp) {
		
		obj->rtc_parse_sdp_answer(sdp);
	}
	
	void rtc_add_ice_candidate(Connection* obj, const char *ice) {

		obj->rtc_add_ice_candidate(ice);
	}
	
	void rtc_del_peer_connection(Connection* obj) {

		obj->rtc_del_peer_connection();
	}	

	void rtc_del_data_channel(Connection* obj) {

		obj->rtc_del_data_channel();
	}	

	void rtc_del_peer_connection_factory(Connection* obj) {

		obj->rtc_del_peer_connection_factory();
	}
	
	int rtc_datachannel_get_state(Connection* obj) {

		obj->rtc_datachannel_get_state();
	}	

	int rtc_datachannel_send_binary(Connection* obj, char *buf, int buf_size) {
		
		obj->rtc_datachannel_send_binary(buf, buf_size);
	}
	
	int rtc_datachannel_send_text(Connection* obj, char *buf, int buf_size) {
		
		obj->rtc_datachannel_send_text(buf, buf_size);
	}
}