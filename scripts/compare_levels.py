import sys, statistics, os, math, textwrap
import matplotlib.pyplot as plt

if len(sys.argv) < 4:
    sys.exit(1)

num_searches = int(sys.argv[1])
output_file = sys.argv[2]
input_files = sys.argv[3:]

def get_stats(file_path):
    if not os.path.exists(file_path):
        return None
    
    with open(file_path, 'r') as f:
        data = [float(line.strip()) for line in f if line.strip()]
    
    if len(data) > 15:
        data = data[10:]
    
    if not data: return None
    if len(data) == 1: return data[0] / num_searches, 0
    
    mu = statistics.mean(data)
    sigma = statistics.stdev(data)
    lower, upper = mu - 2*sigma, mu + 2*sigma

    filtered = [x for x in data if lower <= x <= upper]
    if not filtered: filtered = data
    
    n = len(filtered)
    mean_val = statistics.mean(filtered)
    
    if n > 1:
        sem = statistics.stdev(filtered) / math.sqrt(n)
        # 20 измерений, доверительная вероятность - 0.95
        STUDENTS_COEFF = 2.08596344727
        error = STUDENTS_COEFF * sem
    else:
        error = 0
        
    return mean_val / num_searches, error / num_searches

results = {}
for f_path in input_files:
    res = get_stats(f_path)
    if res is not None:
        label = os.path.basename(f_path).replace('raw_', '').replace('.txt', '')
        results[label] = res

if not results:
    sys.exit(1)

sorted_items = sorted(results.items(), key=lambda item: item[1][0], reverse=True)
names = [item[0] for item in sorted_items]
values = [item[1][0] for item in sorted_items]
errors = [item[1][1] for item in sorted_items]

plt.figure(figsize=(12, 8))
colors = plt.cm.viridis([i/len(names) for i in range(len(names))])

bars = plt.bar(names, values, color=colors, edgecolor='black', alpha=0.8)

for i, bar in enumerate(bars):
    height = bar.get_height()
    err = errors[i]
    rel_err = (err / height * 100) if height > 0 else 0
    
    x_start = bar.get_x()
    x_end = x_start + bar.get_width()
    x_center = x_start + bar.get_width() / 2
    
    plt.vlines(x_center, height - err, height + err, color='red', lw=0.8)
    plt.hlines([height - err, height + err], x_start, x_end, color='red', lw=0.8)
    
    label_text = f'{height:.0f} ± {err:.0f}\n(± {rel_err:.1f}%)'
    plt.text(x_center, height + err, label_text, ha='center', va='bottom', 
             fontweight='bold', fontsize=9)

def calc_percent(v_old, v_new):
    if v_old and v_new:
        diff = (1 - (v_new / v_old)) * 100
        return f"+{diff:.1f}%"
    return "N/A"

stats_parts = []
for i in range(len(names) - 1):
    p = calc_percent(values[i], values[i+1])
    stats_parts.append(f"{names[i]}->{names[i+1]}: {p}")

if len(names) > 1:
    total_p = calc_percent(values[0], values[-1])
    stats_parts.append(f"TOTAL: {total_p}")

stats_text = " | ".join(stats_parts)
wrapped_stats = textwrap.fill(stats_text, width=100)

plt.figtext(0.5, 0.01, wrapped_stats, ha="center", fontsize=9, 
            bbox={"facecolor":"orange", "alpha":0.2, "pad":5})

plt.title("Performance Comparison", fontsize=14)
plt.ylabel("CPU Ticks per Search", fontsize=12)
plt.tight_layout(rect=[0, 0.1, 1, 0.95])
plt.savefig(output_file)
