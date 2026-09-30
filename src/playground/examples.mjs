export const examples = {
  hello: 'main(): Void {\n  print("Hello, browser!")\n}\n',
  exact: 'main(): Void {\n  third = 1/3\n  print((third * 30).nat())\n  print((1/8).dec())\n}\n',
  pure: 'pure square(n: Nat): Nat {\n  n * n\n}\n\nmain(): Void {\n  print(square(12))\n}\n',
  text: 'import "stdlib/text"\n\nmain(): Void {\n  print(text_ends_with("hello.panack", ".panack"))\n}\n',
};
