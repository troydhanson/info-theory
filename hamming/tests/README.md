test1: apply ecc code to dictionary words, reverse ecc coding
test2: apply ecc code to random bytes, reverse ecc coding
test3: apply ecc code to dictionary words, add noise, reverse ecc coding
test4: apply ecc code to random bytes, add noise, reverse ecc coding
test5: apply ecc to "hello, world!", add noise, hexdump it, reverse ecc 
test6: ~test5 but using -x extended (error detecting) codes
test7: ~test6 but with excessive noise, extended coding detects error 
test8: ~test5 but with excessive noise, without extended coding, no error detection
