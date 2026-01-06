#!/usr/bin/env bats

bats_load_library '/usr/lib/bats/bats-support/load.bash'
bats_load_library '/usr/lib/bats/bats-assert/load.bash'

@test "make clean all" {
	run make clean all
	assert_success
}

@test "git restore test.cfg" {
	run git restore test.cfg
	assert_success
}

# Generic failures

@test "help" {
	run ./main --help
	assert_success
}
@test "invalid usage" {
	run ./main
	assert_failure 22
}

# Make sure type detection works

@test "config type string" {
	run ./main test.cfg type test-auto
	assert_output "string"
}

@test "config type float" {
	run ./main test.cfg type test-float
	assert_output "float"
}

@test "config type int" {
	run ./main test.cfg type test-int
	assert_output "int"
}

@test "config type int64" {
	run ./main test.cfg type test-int64
	assert_output "int64"
}

@test "config type bool" {
	run ./main test.cfg type test-bool
	assert_output "bool"
}

# Make read for all types work

@test "config read auto" {
	run ./main test.cfg read auto test-auto
	assert_output "auto works"
}

@test "config read string" {
	run ./main test.cfg read string test-str
	assert_output "hello world"
}

@test "config read int" {
	run ./main test.cfg read int test-int
	assert_output "$((0x1234))"
}

@test "config read int64" {
	run ./main test.cfg read int64 test-int64
	assert_output "576460752303423488"
}

@test "config read float" {
	run ./main test.cfg read float test-float
	assert_output "1.230000"
}

@test "config read bool" {
	run ./main test.cfg read bool test-bool
	assert_output "true"
}

# Test writing

@test "config write bool" {
	run ./main test.cfg write bool test-bool false
	assert_success

	run ./main test.cfg read bool test-bool
	assert_output "false"

	run ./main test.cfg write bool test-bool yes
	assert_success

	run ./main test.cfg read bool test-bool
	assert_output "true"

	run ./main test.cfg write bool test-bool 0
	assert_success

	run ./main test.cfg read bool test-bool
	assert_output "false"

	run ./main test.cfg type test-bool
	assert_output "bool"
}

@test "config write float" {
	run ./main test.cfg write float test-float 2.56
	assert_success

	run ./main test.cfg read float test-float
	assert_output "2.560000"

	run ./main test.cfg write float test-float 1.23e6
	assert_success

	run ./main test.cfg read float test-float
	assert_output "1230000.000000"

	run ./main test.cfg type test-float
	assert_output "float"
}

@test "config write int" {
	run ./main test.cfg write int test-int 123
	assert_success

	run ./main test.cfg read int test-int
	assert_output "123"

	run ./main test.cfg write int test-int 456
	assert_success

	run ./main test.cfg read int test-int
	assert_output "456"

	run ./main test.cfg type test-int
	assert_output "int"
}

@test "config write int64" {
	run ./main test.cfg write int64 test-int64 6969
	assert_success

	run ./main test.cfg read int64 test-int64
	assert_output "6969"

	run ./main test.cfg type test-int64
	assert_output "int64"

	run ./main test.cfg write int64 test-int64 4242
	assert_success

	run ./main test.cfg read int64 test-int64
	assert_output "4242"

	run ./main test.cfg type test-int64
	assert_output "int64"
}

@test "config delete" {
	run ./main test.cfg type test-delete
	assert_output "string"

	run ./main test.cfg delete test-delete
	assert_success

	run ./main test.cfg type test-delete
	assert_failure 6
}

@test "config create auto" {
	run ./main test.cfg type new-auto
	assert_failure 6

	run ./main test.cfg write auto new-auto 7
	assert_failure 95
}

@test "config create int" {
	run ./main test.cfg type new-int
	assert_failure 6

	run ./main test.cfg write int new-int 42
	assert_success

	run ./main test.cfg type new-int
	assert_output "int"

	run ./main test.cfg read auto new-int
	assert_output "42"
}

@test "config create int64" {
	run ./main test.cfg type new-int64
	assert_failure 6

	run ./main test.cfg write int64 new-int64 1099511627776
	assert_success

	run ./main test.cfg type new-int64
	assert_output "int64"

	run ./main test.cfg read auto new-int64
	assert_output "1099511627776"
}

@test "config create float" {
	run ./main test.cfg type new-float
	assert_failure 6

	run ./main test.cfg write float new-float 42.6969
	assert_success

	run ./main test.cfg type new-float
	assert_output "float"

	run ./main test.cfg read auto new-float
	assert_output "42.696900"
}
