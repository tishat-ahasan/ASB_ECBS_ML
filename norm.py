import numpy as np

# Load data from the text file
with open("merged_file.txt", "r") as file:
    data = []
    for line in file:
        line_values = [float(x) for x in line.strip().split() if x]  # Handle empty lines and non-numeric values
        data.append(line_values)

data = np.array(data)

# Check if there are columns with all zeros
zero_variance_columns = np.where(~data.any(axis=0))[0]

# Remove columns with all zeros
data = np.delete(data, zero_variance_columns, axis=1)


# Min-Max Normalization (scale to range 0-1)
min_values = data.min(axis=0)
max_values = data.max(axis=0)
normalized_data = (data - min_values) / (max_values - min_values)

# Handle potential division by zero errors (if a feature has the same value for all samples)
normalized_data = np.nan_to_num(normalized_data, nan=0.0)  # Replace NaN with 0

# Save the normalized data to a new text file
np.savetxt("normalized_data.txt", normalized_data, fmt="%.6f")  # Adjust the format as needed