import csv
import sys
from collections import defaultdict

import matplotlib.pyplot as plt


def load(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append(r)
    return rows


def main():
    if len(sys.argv) < 2:
        print("Usage: plot_bench.py bench.csv")
        return

    rows = load(sys.argv[1])
    if not rows:
        print("No data")
        return

    builds = defaultdict(list)
    queries = defaultdict(list)
    for r in rows:
        backend = r.get("backend", "")
        q = float(r.get("queries", 0))
        build = float(r.get("build_ms", 0))
        query = float(r.get("query_ms", 0))
        builds[backend].append((q, build))
        queries[backend].append((q, query))

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))

    for backend, pts in builds.items():
        pts = sorted(pts)
        ax1.plot([p[0] for p in pts], [p[1] for p in pts], marker="o", label=backend)
    ax1.set_title("Build time")
    ax1.set_xlabel("Queries (count)")
    ax1.set_ylabel("ms")
    ax1.legend()

    for backend, pts in queries.items():
        pts = sorted(pts)
        ax2.plot([p[0] for p in pts], [p[1] for p in pts], marker="o", label=backend)
    ax2.set_title("Query time")
    ax2.set_xlabel("Queries (count)")
    ax2.set_ylabel("ms")
    ax2.legend()

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
