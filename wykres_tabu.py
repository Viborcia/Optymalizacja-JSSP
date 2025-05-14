import pandas as pd
import matplotlib.pyplot as plt

#df = pd.read_csv("harmonogram_random.csv")
df = pd.read_csv("harmonogram_tabu.csv")
fig, ax = plt.subplots(figsize=(12, 6))

# Nowa, poprawna wersja get_cmap (bez dodatkowego argumentu)
colors = plt.colormaps.get_cmap("tab20")

for _, row in df.iterrows():
    ax.barh(
        y=f"Maszyna {row['machine_id']}",
        width=row['end_time'] - row['start_time'],
        left=row['start_time'],
        color=colors(row['job_id'] / df['job_id'].max()),
        edgecolor='black'
    )
    ax.text(row['start_time'] + 0.1, f"Maszyna {row['machine_id']}",
            f"J{row['job_id']}.O{row['operation_id']}",
            fontsize=8, va='center', ha='left')

ax.set_xlabel("Czas")
ax.set_ylabel("Maszyny")
ax.set_title("Wykres Gantta dla harmonogramu JSSP")
plt.tight_layout()
plt.show()
