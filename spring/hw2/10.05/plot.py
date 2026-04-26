import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

here = Path(__file__).parent
csv_path = here / "collisions.csv"

series = defaultdict(list)
with csv_path.open() as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        if len(row) != 3 or not row[1].isdigit():
            continue
        name, sample_size, collisions = row[0], int(row[1]), int(row[2])
        series[name].append((sample_size, collisions))

for name in series:
    series[name].sort()

names = sorted(series.keys())

fig, ax = plt.subplots(figsize=(11, 6))
all_ys = []
for name in names:
    xs = np.array([p[0] for p in series[name]])
    ys = np.array([p[1] for p in series[name]], dtype=float)
    ys_plot = np.where(ys > 0, ys, np.nan)
    ax.loglog(xs, ys_plot, marker="o", label=name)
    all_ys.extend(ys[ys > 0].tolist())

all_xs = sorted({p[0] for s in series.values() for p in s})
x_ref = np.array([all_xs[0], all_xs[-1]])
y_ref = x_ref.astype(float) ** 2 / (2 * 2 ** 32)
ax.loglog(x_ref, y_ref, "k--", linewidth=1, label=r"$N^2 / 2^{33}$ (теор.)")

ax.set_xlim(all_xs[0] * 0.9, all_xs[-1] * 1.2)
ax.set_ylim(0.7, max(all_ys) * 1.5)
ax.set_xlabel("Размер выборки N")
ax.set_ylabel("Число коллизий (log)")
ax.set_title("Коллизии хэш-функций, log–log")
ax.grid(True, which="both", linestyle="--", alpha=0.4)
ax.legend(title="Hash function", bbox_to_anchor=(1.02, 1), loc="upper left")
fig.tight_layout()
fig.savefig(here / "output_loglog.png", dpi=140)

print("saved:", here / "output_loglog.png")
