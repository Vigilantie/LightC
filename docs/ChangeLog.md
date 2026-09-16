# LightC

## 9-16-26
Printing: Added `printnl`. At the top level, `print` now omits the newline and `printnl` includes it. 
- **While loops:** Code generation now reads the condition through `)` and opens the loop body.
- **Lexer refactoring:** Added shared token-buffer growth and token insertion helpers, including allocation-failure handling. Parentheses now receive separate token types.
- **Generated C cleanup:** Changed cleanup to run when `NDEBUG` is defined; its local definition is now commented out.

## 6-8-26
### Features
Cleaned up the project, and updated the README

## 6-6-26
### Features
Added working functions
Added C integration directly into the language

## 3-16-26
### Features
Supports basic vairables, print, input, and arithmetic
