import sys, statistics
import matplotlib.pyplot as plt
import os

REPORTS_DIR = "reports"

if len(sys.argv) < 3:
    print("Usage: python3 compare_levels.py <num_searches> <output.png>")
    sys.exit(1)

num_searches = int(sys.argv[1])
output_file = sys.argv[2]

levels = ["DEFAULT", "LEVEL0", "LEVEL1", "LEVEL2"]

def get_median(level_name):
    file_path = f"{REPORTS_DIR}/raw_{level_name}.txt"
    if not os.path.exists(file_path):
        return None
    
    with open(file_path, 'r') as f:
        data = [float(line.strip()) for line in f if line.strip()]
    
    if len(data) > 15:
        data = data[10:]
    
    mu = statistics.mean(data)
    sigma = statistics.stdev(data)
    lower, upper = mu - 2*sigma, mu + 2*sigma
    
    filtered = [x for x in data if lower <= x <= upper]
    
    return statistics.median(filtered) / num_searches

results = {}
for lvl in levels:
    val = get_median(lvl)
    if val is not None:
        results[lvl] = val

names = []
values = []
for lvl in levels:
    if lvl in results:
        names.append(lvl)
        values.append(results[lvl])

if not names:
    print("[ERROR] No data found for any level in reports/ directory.")
    sys.exit(1)

plt.figure(figsize=(10, 7))
colors = plt.cm.viridis([i/len(names) for i in range(len(names))])
bars = plt.bar(names, values, color=colors, edgecolor='black', alpha=0.8)

for bar in bars:
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height,
             f'{height:.2f}', ha='center', va='bottom', fontweight='bold')

def calc_percent(v_old, v_new):
    if v_old and v_new:
        diff = (1 - (v_new / v_old)) * 100
        return f"+{diff:.1f}%"
    return "N/A"

p1 = calc_percent(results.get("DEFAULT"), results.get("LEVEL0"))
p2 = calc_percent(results.get("LEVEL0"), results.get("LEVEL1"))
p3 = calc_percent(results.get("LEVEL1"), results.get("LEVEL2"))

final_diff = calc_percent(results.get("LEVEL0"), results.get("LEVEL2"))

stats_text = (f"Def->L0: {p1} | L0->L1: {p2} | "
              f"L1->L2: {p3} | TOTAL (L0->L2): {final_diff}")

plt.figtext(0.5, 0.01, stats_text, ha="center", fontsize=9, 
            bbox={"facecolor":"orange", "alpha":0.2, "pad":5})

plt.title("Performance Comparison: Improvement %", fontsize=14)
plt.ylabel("CPU Ticks (Lower is Better)", fontsize=12)

plt.tight_layout(rect=[0, 0.05, 1, 0.95])
plt.savefig(output_file)
