import matplotlib.pyplot as plt
import numpy as np

# Load data
sample, cache, dram = np.loadtxt("latencies.txt", skiprows=1, unpack=True)

# Plot
plt.plot(sample, cache, label="Cache", marker='o')
plt.plot(sample, dram, label="DRAM", marker='o')
plt.xlabel("Sample Number")
plt.ylabel("Latency (cycles)")
plt.title("Cache vs DRAM Access Latency")
plt.legend()
plt.grid(True)
plt.tight_layout()

# Save the plot as a PNG file
plt.savefig("latencies_plot.png")

# Show the plot
plt.show()
