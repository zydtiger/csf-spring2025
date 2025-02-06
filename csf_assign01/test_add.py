number1_buf = [0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0, 0]
number2_buf = [1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0, 0]
result = [0] * 8

for i in range(8):
  result[i] = number1_buf[i] + number2_buf[i]
  
print(result)

carry = 0
for i in range(8):
  if carry > 0:
    result[i] += carry
    carry = 0
  if result[i] > 0xFFFFFFFF:
    carry = (result[i] >> 32)
    result[i] &= 0xFFFFFFFF

print(result)