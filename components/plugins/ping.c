#include "ping.h"

PING_RESULTS ping_results;

static void ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    float avg;
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    ESP_LOGI(TAG_PING,"%d bytes from %s icmp_seq=%d ttl=%d time=%d ms",
           recv_len, ipaddr_ntoa((ip_addr_t*)&target_addr), seqno, ttl, elapsed_time);
    
}

static void ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    ESP_LOGW(TAG_PING,"From %s icmp_seq=%d timeout",ipaddr_ntoa((ip_addr_t*)&target_addr), seqno);
}

static void ping_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
	uint32_t gap_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
	esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &gap_time_ms, sizeof(gap_time_ms));
    uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    if (IP_IS_V4(&target_addr)) {
        ESP_LOGI(TAG_PING,"--- %s ping statistics ---",inet_ntoa(*ip_2_ip4(&target_addr)));
    } else {
        ESP_LOGI(TAG_PING,"--- %s ping statistics ---",inet6_ntoa(*ip_2_ip6(&target_addr)));
    }
    ESP_LOGI(TAG_PING,"%d packets transmitted, %d received, %d%% packet loss, time %dms, time gap %dms", transmitted, received, loss, total_time_ms,gap_time_ms);
    ping_results.total_time_ms = total_time_ms;
    ping_results.loss = loss;
    ping_results.avg_time_ms = gap_time_ms;
    // delete the ping sessions, so that we clean up all resources and can create a new ping session
    // we don't have to call delete function in the callback, instead we can call delete function from other tasks
    esp_ping_delete_session(hdl);
}

int do_ping()
{
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();

    // int nerrors = arg_parse(argc, argv, (void **)&ping_args);
    // if (nerrors != 0) {
    //     arg_print_errors(stderr, ping_args.end, argv[0]);
    //     return 1;
    // }

    // if (ping_args.timeout->count > 0) {
    //     config.timeout_ms = (uint32_t)(ping_args.timeout->dval[0] * 1000);
    // }

    // if (ping_args.interval->count > 0) {
    //     config.interval_ms = (uint32_t)(ping_args.interval->dval[0] * 1000);
    // }

    // if (ping_args.data_size->count > 0) {
    //     config.data_size = (uint32_t)(ping_args.data_size->ival[0]);
    // }

    // if (ping_args.count->count > 0) {
    //     config.count = (uint32_t)(ping_args.count->ival[0]);
    // }

    // if (ping_args.tos->count > 0) {
    //     config.tos = (uint32_t)(ping_args.tos->ival[0]);
    // }

    // parse IP address
    struct sockaddr_in6 sock_addr6;
    ip_addr_t target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    
    if (inet_pton(AF_INET6, "google.com", &sock_addr6.sin6_addr) == 1) {
        /* convert ip6 string to ip6 address */
        ipaddr_aton("google.com", &target_addr);
    } else {
        struct addrinfo hint;
        struct addrinfo *res = NULL;
        memset(&hint, 0, sizeof(hint));
        /* convert ip4 string or hostname to ip4 or ip6 address */
        if (getaddrinfo( "google.com", NULL, &hint, &res) != 0) {
            ESP_LOGW(TAG_PING,"ping: unknown host %s", "google.com");
            return 1;
        }
        if (res->ai_family == AF_INET) {
            struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
            inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
        } else {
            struct in6_addr addr6 = ((struct sockaddr_in6 *) (res->ai_addr))->sin6_addr;
            inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
        }
        freeaddrinfo(res);
    }
    config.target_addr = target_addr;

    /* set callback functions */
    esp_ping_callbacks_t cbs = {
        .on_ping_success = ping_on_ping_success,
        .on_ping_timeout = ping_on_ping_timeout,
        .on_ping_end = ping_on_ping_end,
        .cb_args = NULL
    };
    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);

    return 0;
}
/**/