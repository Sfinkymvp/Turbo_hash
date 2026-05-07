import sys, csv, statistics
import matplotlib.pyplot as plt

if len(sys.argv) < 3:
    print("Usage: python3 plot_monitor.py <input.csv> <output.png>")
    sys.exit(1)

timestamps, freqs, temps = [], [], []
with open(sys.argv[1], 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        timestamps.append(float(row['timestamp']))
        freqs.append(float(row['freq_mhz']))
        temps.append(float(row['temp_c']))

time_axis = [ts - timestamps[0] for ts in timestamps]

avg_f, std_f = statistics.mean(freqs), statistics.stdev(freqs)
avg_t, std_t = statistics.mean(temps), statistics.stdev(temps)

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), sharex=True)

ax1.plot(time_axis, freqs, color='blue', alpha=0.6, label='Frequency')
ax1.axhline(avg_f, color='darkblue', linestyle='--', label=f'Mean: {avg_f:.1f} MHz')
ax1.fill_between(time_axis, avg_f - std_f, avg_f + std_f, color='blue', alpha=0.1, label='Std freq deviation')
ax1.set_ylabel('Frequency (MHz)')
ax1.set_title(f"Performance: {sys.argv[1]}")
ax1.legend(loc='upper right', fontsize='small')
ax1.grid(True, alpha=0.2)

ax2.plot(time_axis, temps, color='red', linewidth=1.5, label='Temperature')
ax2.axhline(avg_t, color='darkred', linestyle='--', label=f'Mean: {avg_t:.1f} C')
ax2.fill_between(time_axis, avg_t - std_t, avg_t + std_t, color='red', alpha=0.15, label='Std temp deviation')
ax2.set_xlabel('Time (seconds)')
ax2.set_ylabel('Temperature (C)')
ax2.legend(loc='upper right', fontsize='small')
ax2.grid(True, alpha=0.2)

plt.tight_layout()
plt.savefig(sys.argv[2])
