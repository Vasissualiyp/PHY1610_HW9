import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
data = pd.read_csv('timing_output.csv')

# Declare the serial time variable
serial_time = 888

# Calculate speedup and add it to the DataFrame
data['speedup'] = serial_time / data['time']

# Plot the speedup as a function of the number of processes
plt.plot(data['number_of_proccesses'], data['speedup'], marker='o', linestyle='-')
plt.xlabel('Number of Processes')
plt.ylabel('Speedup')
plt.title('Speedup vs. Number of Processes')
plt.grid()

# Save the plot to a file
plt.savefig('speedup_plot.png')

# Show the plot
plt.show()

