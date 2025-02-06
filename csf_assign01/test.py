def convert_to_u256(hex_number):
    # Ensure the hex number is exactly 64 digits (256 bits)
    hex_number = hex_number[:64].zfill(64)  # Truncate or pad with leading zeros
    
    # Split the hex number into 8 chunks of 8 digits each (32 bits each)
    u32_chunks = [hex_number[i:i+8] for i in range(0, 64, 8)]
    
    # Convert each chunk to an integer (optional, if you want numeric values instead of hex strings)
    u32_chunks = [int(chunk, 16) for chunk in u32_chunks]
    
    return u32_chunks[::-1]

# Input hexadecimal number
hex_number = "2C4A1B8F9E4F7D2D4E62C4A1B8F9E4F7D2D4E6"

# Convert to u256
u256_representation = convert_to_u256(hex_number)

# Print the result
print("u256 Representation (8 u32 units):")
for i, chunk in enumerate(u256_representation):
    print(f"u32[{i}]: {chunk}")
