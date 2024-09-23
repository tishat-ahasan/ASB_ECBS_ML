#!/bin/bash

# Set the number of runs you want
num_runs=100

# Create a directory to store the outputs if it doesn't exist
output_dir="mapf_outputs"
mkdir -p "$output_dir"

# Main loop for running the code multiple times
for (( i=1; i<=num_runs; i++ ))
do
    # Generate a random seed value 
    seed=$((RANDOM % 1000000))  # Adjust the range if needed

    # Update the parameter file with the new seed
    sed -i "s/^seed.*/seed=$seed/" sample-param.txt

    # Compile and run your C code (assuming 'make crun' does both)
    make crun param=sample-param.txt

    # Store the output in the designated directory (modify if needed)
    mv output.txt "$output_dir/output_$i.txt" 
done

echo "All $num_runs runs completed. Outputs stored in $output_dir"