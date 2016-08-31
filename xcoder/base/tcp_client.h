#ifndef _TCP_CLIENT_H__
#define _TCP_CLIENT_H__

#include <array>

#include "network_loop.h"
#include "utils.h"
#include "types.h"

namespace agora { namespace base {

    class tcp_client;
    /**
     * @brief The tcp_client_listener struct
     */
    struct tcp_client_listener
    {
        virtual void on_connect(tcp_client* client, bool connected) = 0;
        virtual void on_packet(tcp_client* client, unpacker & p, uint16_t server_type, uint16_t uri) = 0;
        virtual void on_ping_cycle(tcp_client* client) = 0;
        virtual void on_socket_error(tcp_client* client) = 0;
    };

    /**
     * @brief The tcp_client class
     * @note base class for tcp client. provide connect/send method and callback handler accordingly.
     */
    class tcp_client
        : public connect_listener
        , public tcp_listener
        , public timer_listener
    {
        enum {
            SERVER_TIMEOUT    = 10,
            PING_INTERVAL    = 3,
            NET_BUFFER_SIZE    = 64 * 1024
        };

        enum session_status {
            NOT_CONNECTED_STATUS = 0,
            CONNECTING_STATUS = 1,
            CONNECTED_STATUS = 2,
        };

        /**
         * @brief The connection_info struct
         * @note remote endpoint info. last_active and last_ping need to be calculated dedicatedly to keep it alive
         */
        struct connection_info
        {
            address_info    address;
            bufferevent*    handle;
            uint32_t      last_active;
            session_status  status;
            uint32_t    last_ping;

            explicit connection_info(uint32_t ip, uint16_t port)
                : address(ip, port)
                , handle(nullptr)
                , last_active(0)
                , status(NOT_CONNECTED_STATUS)
                , last_ping(0)
            {}

            void activate() { last_active = now_seconds(); }
            bool time_to_ping(int now) { return now - last_ping > PING_INTERVAL; }
        };

    public:
        explicit tcp_client(network_loop& net, uint32_t ip, uint16_t port, tcp_client_listener* listener, bool keep_alive=true, uint32_t timeout = SERVER_TIMEOUT);
        virtual ~tcp_client();

        virtual void on_connect(bufferevent* bev, short events);
        virtual void on_read(bufferevent* bev);
        virtual void on_write(bufferevent* /*bev*/) {}
        virtual void on_event(bufferevent* bev, short events);
        virtual void on_timer();

        void set_timeout(uint32_t timeout) { timeout_ = timeout; }
        bool is_connected() const;
        bool connect();
        void close();
        bool send_message(const packet& p);
        bool send_buffer(const char* data, uint32_t length);
        std::string remote_addr() const;
        const address_info & remote_socket_address() const { return conn_info_.address; }
        uint32_t remote_ip() const;
        uint16_t remote_port() const;

    private:
        void check_connection(uint32_t now);
        void check_ping(uint32_t now);
        void on_data(bufferevent* bev, const char* data, size_t length);
        void do_close();

    protected:
        network_loop& net_;
        std::array<char, NET_BUFFER_SIZE> buffer_;
        event* timer_;
    public:
        connection_info  conn_info_;
    private:
        tcp_client_listener* listener_;
        bool stop_;
        uint32_t timeout_;
        bool keep_alive_;
    };

} // namespace base
} // namespace agora

#endif // _TCP_CLIENT_H__
