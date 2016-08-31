#include "tcp_client.h"
#include "log.h"
#include "packer.h"
#include "packet.h"
using namespace agora::base;
using namespace std;

tcp_client::tcp_client(network_loop &net, uint32_t ip, uint16_t port, tcp_client_listener* listener, bool keep_alive, uint32_t timeout)
    : net_(net)
    , buffer_()
    , timer_(nullptr)
    , conn_info_(ip, port)
    , listener_(listener)
    , stop_(true)
    , timeout_(timeout)
    , keep_alive_(keep_alive)
{
    timer_ = net_.add_timer(500000, this);
}

tcp_client::~tcp_client()
{
    net_.remove_timer(timer_);
    close();
}

bool tcp_client::is_connected() const
{
    return conn_info_.status == tcp_client::CONNECTED_STATUS;
}

void tcp_client::on_connect(bufferevent *bev, short events)
{
    conn_info_.activate();
    if (events & BEV_EVENT_CONNECTED) {
        net_.set_tcp_listener(bev, this);
        conn_info_.status = CONNECTED_STATUS;
        listener_->on_connect(this, true);
    } else if (events & BEV_EVENT_ERROR) {
        log(ERROR_LOG, "connection error %x on socket %u @ %p @ %s", events, bufferevent_getfd(bev), bev, remote_addr().c_str());
        listener_->on_connect(this, false);
        do_close();
    }
}

void tcp_client::on_read(bufferevent *bev)
{
    evbuffer * input = bufferevent_get_input(bev);
    uint16_t packet_length = 0;  // packet length 2 bytes, we don't accept packet large than 64K
    while (true) {
        size_t length = evbuffer_get_length(input);
        if (length <= 2) {
            break;
        }
        evbuffer_copyout(input, &packet_length, sizeof(packet_length));
        if (length < packet_length) {
            break;
        }
        bufferevent_read(bev, buffer_.data(), packet_length);
        on_data(bev, buffer_.data(), packet_length);
    }
}

void tcp_client::on_event(bufferevent *bev, short events)
{
    int s = bufferevent_getfd(bev);
    if (events & BEV_EVENT_CONNECTED) {
        log(NOTICE_LOG, "socket %u %s connected", s, remote_addr().c_str());
    } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
        log(NOTICE_LOG, "socket %u %s error %x", s, remote_addr().c_str(), events);
        listener_->on_socket_error(this);
        do_close();
    }
}

void tcp_client::on_timer()
{
    uint32_t now = now_seconds();
    check_ping(now);
    check_connection(now);
}

bool tcp_client::connect()
{
    if (conn_info_.status != NOT_CONNECTED_STATUS) {
        log(INFO_LOG, "ignore tcp_client connect @ status %u", conn_info_.status);
        return true;
    }
    ///@note activate must be placed before stop_=false
    conn_info_.status = CONNECTING_STATUS;
    conn_info_.activate();
    stop_ = false;
    conn_info_.handle = net_.tcp_connect(conn_info_.address.ip, conn_info_.address.port, this);
    if (conn_info_.handle) {
      LOG(NOTICE, "connecting to %s handle %x, %u", remote_addr().c_str(), conn_info_.handle,
          bufferevent_getfd(conn_info_.handle));
      return true;
    }

    LOG(ERROR, "Failed to connect %s, Reason: %m", remote_addr().c_str());
    return false;
}

bool tcp_client::send_message(const packet &p)
{
    if (!is_connected()) {
        LOG(ERROR, "cannot send message %u %u to %s, not connected", p.server_type, p.uri, remote_addr().c_str());
        return false;
    }
    net_.send_message(conn_info_.handle, p);
    return true;
}

bool tcp_client::send_buffer(const char *data, uint32_t length)
{
    if (!is_connected()) {
        LOG(ERROR, "cannot send buffer %u to %s, not connected", length, remote_addr().c_str());
        return false;
    }

    net_.send_buffer(conn_info_.handle, data, length);
    return true;
}

string tcp_client::remote_addr() const
{
    return address_to_string(conn_info_.address.ip, htons(conn_info_.address.port));
}

uint32_t tcp_client::remote_ip() const
{
    return conn_info_.address.ip;
}

uint16_t tcp_client::remote_port() const
{
    return conn_info_.address.port;
}

void tcp_client::on_data(bufferevent *bev, const char *data, size_t length)
{
    (void)bev;
    conn_info_.activate();

    unpacker p(data, length);
    p.pop_uint16(); // uint16_t packet_length
    uint16_t server_type = p.pop_uint16();
    uint16_t uri = p.pop_uint16();
    p.rewind();

    try {
        listener_->on_packet(this, p, server_type, uri);
    } catch (std::overflow_error & e) {
        log(WARN_LOG, "error on packet %u from %s %s", uri, remote_addr().c_str(), e.what());
    }
}

void tcp_client::do_close()
{
    conn_info_.handle = nullptr;
    conn_info_.status = tcp_client::NOT_CONNECTED_STATUS;
}

void tcp_client::check_ping(uint32_t now)
{
    if (conn_info_.status < CONNECTED_STATUS) {
        return;
    }
    if (conn_info_.time_to_ping(now)) {
        listener_->on_ping_cycle(this);
        conn_info_.last_ping = now_seconds();
    }
}

void tcp_client::check_connection(uint32_t now)
{
    if (stop_) {
        log(INFO_LOG, "TCP connection to %s stopped", remote_addr().c_str());
        return;
    }

    if (!keep_alive_ || now - conn_info_.last_active <= timeout_) {
        return;
    }
    log(WARN_LOG, "TCP connection to %s timeout since %u now %u", remote_addr().c_str(), conn_info_.last_active, now);
    if (conn_info_.handle != nullptr) {
        log(INFO_LOG, "close timeout connection %x %s", conn_info_.handle, remote_addr().c_str());
        listener_->on_connect(this, false);
        net_.close(conn_info_.handle);
        do_close();
    }
    connect();
}

void tcp_client::close()
{
    stop_ = true;
    net_.close(conn_info_.handle);
    do_close();
}
