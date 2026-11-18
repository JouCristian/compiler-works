#!/bin/sh
set -eu

root=$(pwd -P)
compiler="$root/build/compiler"
example="$root/docs/semantic-analysis/results/example.c"
expected="$root/docs/semantic-analysis/results/example-ir.txt"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work"

mkdir -p docs/lexical-analysis docs/syntax-analysis/results docs/semantic-analysis/results
ir=docs/semantic-analysis/results/quadruples.txt
syntax_log=docs/syntax-analysis/results/syntax-errors.txt

assert_failure() {
	name=$1
	source=$2
	message=$3
	if "$compiler" "$source" > "$name.out" 2> "$name.err"; then
		printf 'FAIL: %s returned success\n' "$name" >&2
		exit 1
	fi
	grep -Fq "$message" "$name.err"
	if grep -Eq 'Quadruples|IR saved to|Grammar analysis successfully' "$name.out"; then
		printf 'FAIL: %s printed a successful IR result\n' "$name" >&2
		exit 1
	fi
}

printf '%s\n' '{ int a; write 999999999999999999999; write a; }' > semantic.c
assert_failure semantic semantic.c 'Use of uninitialized variable'
test ! -e "$ir"
test ! -e "$syntax_log"
if grep -Eq 'IR generation|Grammar error' semantic.err; then exit 1; fi
printf '%s\n' 'PASS: CLI stops before IR when semantic checks fail'

"$compiler" "$example" --all-tokens > example.out 2> example.err
test ! -s example.err
grep -Fq 'Semantic Errors: 0' example.out
grep -Fq 'IR saved to' example.out
sed -n '/^[0-9][0-9]* (/p' example.out > stdout-ir.txt
diff -u "$expected" "$ir"
diff -u "$expected" stdout-ir.txt
printf '%s\n' 'PASS: CLI TokenBuffer to IR matches the full sample on stdout and disk'

printf '%s\n' '{ write 12abc; }' > lexical.c
assert_failure lexical lexical.c 'Parser Skipped:'
cmp "$expected" "$ir"
test ! -e "$syntax_log"
printf '%s\n' 'PASS: CLI lexical errors preserve the previous complete IR'

printf '%s\n' '{ while (1 < 2) write 1; write 999999999999999999999; }' > overflow.c
assert_failure overflow overflow.c 'IR generation error at line 1: Integer literal out of int range'
cmp "$expected" "$ir"
test ! -e "$syntax_log"
printf '%s\n' 'PASS: CLI IR errors preserve the previous complete IR and skip the syntax log'

assert_failure syntax "$root/tests/parser/invalid.c" 'Grammar error at line 3:'
cmp "$expected" "$ir"
grep -Fq 'invalid.c:3:' "$syntax_log"
printf '%s\n' 'PASS: CLI grammar errors retain the first-error log behavior'

mv "$ir" previous-ir.txt
mkdir "$ir"
assert_failure save "$example" 'replace docs/semantic-analysis/results/quadruples.txt'
test -d "$ir"
test -z "$(find "$(dirname "$ir")" -name 'quadruples.txt.*' -print)"
cmp "$expected" previous-ir.txt
printf '%s\n' 'PASS: CLI reports save failure and removes the temporary output'
