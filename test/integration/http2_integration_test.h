#pragma once

#include "test/common/http/http2/http2_frame.h"
#include "test/integration/http_integration.h"

#include "gtest/gtest.h"

using Envoy::Http::Http2::Http2Frame;

namespace Envoy {
class Http2IntegrationTest : public testing::TestWithParam<Network::Address::IpVersion>,
                             public HttpIntegrationTest {
public:
  Http2IntegrationTest() : HttpIntegrationTest(Http::CodecClient::Type::HTTP2, GetParam()) {}

  void SetUp() override { setDownstreamProtocol(Http::CodecClient::Type::HTTP2); }

  void simultaneousRequest(int32_t request1_bytes, int32_t request2_bytes);
};

class Http2RingHashIntegrationTest : public Http2IntegrationTest {
public:
  Http2RingHashIntegrationTest();

  ~Http2RingHashIntegrationTest() override;

  void createUpstreams() override;

  void sendMultipleRequests(int request_bytes, Http::TestHeaderMapImpl headers,
                            std::function<void(IntegrationStreamDecoder&)> cb);

  std::vector<FakeHttpConnectionPtr> fake_upstream_connections_;
  int num_upstreams_ = 5;
};

class Http2MetadataIntegrationTest : public Http2IntegrationTest {
public:
  void SetUp() override {
    config_helper_.addConfigModifier(
        [&](envoy::config::bootstrap::v2::Bootstrap& bootstrap) -> void {
          RELEASE_ASSERT(bootstrap.mutable_static_resources()->clusters_size() >= 1, "");
          auto* cluster = bootstrap.mutable_static_resources()->mutable_clusters(0);
          cluster->mutable_http2_protocol_options()->set_allow_metadata(true);
        });
    config_helper_.addConfigModifier(
        [&](envoy::config::filter::network::http_connection_manager::v2::HttpConnectionManager& hcm)
            -> void { hcm.mutable_http2_protocol_options()->set_allow_metadata(true); });
    setDownstreamProtocol(Http::CodecClient::Type::HTTP2);
    setUpstreamProtocol(FakeHttpConnection::Type::HTTP2);
  }
};

class Http2FloodMitigationTest : public testing::TestWithParam<Network::Address::IpVersion>,
                                 public HttpIntegrationTest {
public:
  Http2FloodMitigationTest() : HttpIntegrationTest(Http::CodecClient::Type::HTTP2, GetParam()) {}

protected:
  void startHttp2Session();
  void floodServer(const Http2Frame& frame);
  void floodServer(absl::string_view host, absl::string_view path,
                   Http2Frame::ResponseStatus expected_http_status);
  Http2Frame readFrame();
  void sendFame(const Http2Frame& frame);
  void setNetworkConnectionBufferSize();
  void beginSession();

  IntegrationTcpClientPtr tcp_client_;
};
} // namespace Envoy
