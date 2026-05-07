int printf(const char *fmt, ...);

int main(void) {
	int written = printf(
		"x=%x o=%o b=%b d=%d c=%c s=%s | "
		"x=%x o=%o b=%b d=%d c=%c s=%s | "
		"zeroes: %x %o %b %d c=%c s=%s | %% done\n"
		"%d %s %x %d%%%c%b\n",
		0x12abu,      01252u,      13u,         -12345,      'A',   "alpha",
		0xdeadbeefu,  0777u,       42u,         67890,       'B',   "beta",
		0u,           0u,          0u,          0,           'C',   "gamma",
		-1, "love", 3802, 100, 33, 126
	);

	printf("secret%chahaha\n", 0x0d);
	printf("what's%cup boys\n", 0x0a);
	printf("string: [%s]\n", "hello");
	printf("long: [%s]\n",
		"this string is deliberately longer than the internal fixed buffer so it "
		"must be emitted in several chunks without breaking formatting");

	return written < 0;
}
