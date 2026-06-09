# LightC How To Docs

## Functions

Function parameters are declared inside parentheses and do not need separate
`let` declarations. Their types are inferred from function calls.

```light
func show(message, count)
let next_count = count + 1
print message
print next_count
end

show("hello", 3)
```

## Direct C

Use a raw C block to emit C statements directly into the current Light
function or main body:

```light
"C"
printf("called from C\n");
end
```
