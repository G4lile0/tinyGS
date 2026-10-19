#include "doctest.h"

#include <initializer_list>

#include "golay24.h"

TEST_CASE("Golay-24 accepts valid codewords") {
  uint32_t zero = 0;
  CHECK(decode_golay24(&zero) == 0);
  CHECK(zero == 0u);

  uint32_t ones = 0xFFFFFF;
  CHECK(decode_golay24(&ones) == 0);
  CHECK(ones == 0xFFFFFFu);
}

TEST_CASE("Golay-24 corrects single-bit errors") {
  for (unsigned bit : {0u, 5u, 11u, 12u, 18u, 23u}) {
    uint32_t word = 1u << bit;
    CAPTURE(bit);
    CHECK(decode_golay24(&word) == 1);
    CHECK(word == 0u);
  }
}

TEST_CASE("Golay-24 corrects two-bit errors") {
  uint32_t word = (1u << 0) | (1u << 23);
  CHECK(decode_golay24(&word) == 2);
  CHECK(word == 0u);
}

TEST_CASE("Golay-24 rejects an uncorrectable word") {
  uint32_t word = 0x00000F;  // weight 4: outside the correction radius
  CHECK(decode_golay24(&word) == -1);
  CHECK(word == 0x00000F);
}
