#include <catch.hpp>

#include "channel.h"

TEST_CASE("The occupancies of an empty channel are zero") {
  champsim::channel uut{};

  CHECK(uut.rq_occupancy() == 0);
  CHECK(uut.wq_occupancy() == 0);
  CHECK(uut.pq_occupancy() == 0);
  CHECK(uut.iq_occupancy() == 0);
}

TEST_CASE("Adding something to the channel's RQ increases its occupancy") {
  champsim::channel uut{};

  champsim::channel::request_type packet{};
  packet.address = champsim::address{0xdeadbeef};
  uut.add_rq(packet);

  CHECK(uut.rq_occupancy() == 1);
  CHECK(uut.wq_occupancy() == 0);
  CHECK(uut.pq_occupancy() == 0);
  CHECK(uut.iq_occupancy() == 0);
}

TEST_CASE("Adding something to the channel's WQ increases its occupancy") {
  champsim::channel uut{};

  champsim::channel::request_type packet{};
  packet.address = champsim::address{0xdeadbeef};
  uut.add_wq(packet);

  CHECK(uut.rq_occupancy() == 0);
  CHECK(uut.wq_occupancy() == 1);
  CHECK(uut.pq_occupancy() == 0);
  CHECK(uut.iq_occupancy() == 0);
}

TEST_CASE("Adding something to the channel's PQ increases its occupancy") {
  champsim::channel uut{};

  champsim::channel::request_type packet{};
  packet.address = champsim::address{0xdeadbeef};
  uut.add_pq(packet);

  CHECK(uut.rq_occupancy() == 0);
  CHECK(uut.wq_occupancy() == 0);
  CHECK(uut.pq_occupancy() == 1);
  CHECK(uut.iq_occupancy() == 0);
}

TEST_CASE("Adding something to the channel's IQ increases its occupancy") {
  champsim::channel uut{};

  champsim::channel::request_type packet{};
  packet.address = champsim::address{0xdeadbeef};
  uut.add_iq(packet);

  CHECK(uut.rq_occupancy() == 0);
  CHECK(uut.wq_occupancy() == 0);
  CHECK(uut.pq_occupancy() == 0);
  CHECK(uut.iq_occupancy() == 1);
}

TEST_CASE("A const channel can return its RQ size") {
  auto rq_size = GENERATE(as<std::size_t>(), 1, 2, 8, 32, 256);
  const champsim::channel uut{rq_size, 32, 32, 32, champsim::data::bits{}, false};

  REQUIRE(uut.rq_size() == rq_size);
}

TEST_CASE("A const channel can return its WQ size") {
  auto wq_size = GENERATE(as<std::size_t>(), 1, 2, 8, 32, 256);
  const champsim::channel uut{32, 32, wq_size, 32, champsim::data::bits{}, false};

  REQUIRE(uut.wq_size() == wq_size);
}

TEST_CASE("A const channel can return its PQ size") {
  auto pq_size = GENERATE(as<std::size_t>(), 1, 2, 8, 32, 256);
  const champsim::channel uut{32, pq_size, 32, 32, champsim::data::bits{}, false};

  REQUIRE(uut.pq_size() == pq_size);
}

TEST_CASE("A const channel can return its IQ size") {
  auto iq_size = GENERATE(as<std::size_t>(), 1, 2, 8, 32, 256);
  const champsim::channel uut{32, 32, 32, iq_size, champsim::data::bits{}, false};

  REQUIRE(uut.iq_size() == iq_size);
}
