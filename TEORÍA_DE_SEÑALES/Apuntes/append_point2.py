import json
from pathlib import Path
path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')

with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

# Create or replace the last empty code cell before the final markdown cell
if len(data['cells']) >= 2 and data['cells'][-2]['cell_type'] == 'code' and (not data['cells'][-2].get('source') or all(not line.strip() for line in data['cells'][-2].get('source'))):
    code_cell = data['cells'][-2]
    code_cell['source'] = [
        '# Punto 2: desarrollo de BinDetection_Lab (1).pdf\n',
        '# Calculamos el umbral MAP y las probabilidades de error para varios valores de pi1.\n',
        'pi1_values = np.array([0.1, 0.3, 0.5, 0.7, 0.9])\n',
        'pi0_values = 1 - pi1_values\n',
        '\n',
        'gamma_map_values = (a0 + a1) / 2 + (sigma_eta**2 / (a1 - a0)) * np.log(pi0_values / pi1_values)\n',
        '\n',
        'PFA_values = 1 - stats.norm.cdf(gamma_map_values, loc=a0, scale=sigma_eta)\n',
        'PM_values = stats.norm.cdf(gamma_map_values, loc=a1, scale=sigma_eta)\n',
        'Pe_values = pi0_values * PFA_values + pi1_values * PM_values\n',
        '\n',
        'print("Tabla de resultados para diferentes valores de pi1:")\n',
        'print("pi1    gamma_MAP      P_FA        P_M         P_e")\n',
        'for pi1, gamma_map_i, PFA_i, PM_i, Pe_i in zip(pi1_values, gamma_map_values, PFA_values, PM_values, Pe_values):\n',
        '    print(f"{pi1:0.1f}   {gamma_map_i:0.6f}   {PFA_i:0.6f}   {PM_i:0.6f}   {Pe_i:0.6f}")\n',
        '\n',
        'plt.figure(figsize=(8, 4))\n',
        'plt.plot(pi1_values, gamma_map_values, marker="o", linestyle="-", label=r"$\gamma_{MAP}$")\n',
        'plt.xlabel(r"Prior $\pi_1$")\n',
        'plt.ylabel(r"Umbral $\gamma_{MAP}$")\n',
        'plt.title(r"Umbral MAP vs. prior $\pi_1$")\n',
        'plt.grid(True)\n',
        'plt.legend()\n',
        'plt.show()\n',
        '\n',
        'plt.figure(figsize=(8, 4))\n',
        'plt.plot(pi1_values, PFA_values, marker="o", label=r"$P_{FA}$")\n',
        'plt.plot(pi1_values, PM_values, marker="s", label=r"$P_{M}$")\n',
        'plt.plot(pi1_values, Pe_values, marker="^", label=r"$P_{e}$")\n',
        'plt.xlabel(r"Prior $\pi_1$")\n',
        'plt.ylabel("Probabilidad")\n',
        'plt.title("Probabilidades de error vs. prior")\n',
        'plt.grid(True)\n',
        'plt.legend()\n',
        'plt.show()\n',
    ]
else:
    data['cells'].insert(-1, {
        'cell_type': 'code',
        'execution_count': None,
        'metadata': {},
        'outputs': [],
        'source': [
            '# Punto 2: desarrollo de BinDetection_Lab (1).pdf\n',
            '# Calculamos el umbral MAP y las probabilidades de error para varios valores de pi1.\n',
            'pi1_values = np.array([0.1, 0.3, 0.5, 0.7, 0.9])\n',
            'pi0_values = 1 - pi1_values\n',
            '\n',
            'gamma_map_values = (a0 + a1) / 2 + (sigma_eta**2 / (a1 - a0)) * np.log(pi0_values / pi1_values)\n',
            '\n',
            'PFA_values = 1 - stats.norm.cdf(gamma_map_values, loc=a0, scale=sigma_eta)\n',
            'PM_values = stats.norm.cdf(gamma_map_values, loc=a1, scale=sigma_eta)\n',
            'Pe_values = pi0_values * PFA_values + pi1_values * PM_values\n',
            '\n',
            'print("Tabla de resultados para diferentes valores de pi1:")\n',
            'print("pi1    gamma_MAP      P_FA        P_M         P_e")\n',
            'for pi1, gamma_map_i, PFA_i, PM_i, Pe_i in zip(pi1_values, gamma_map_values, PFA_values, PM_values, Pe_values):\n',
            '    print(f"{pi1:0.1f}   {gamma_map_i:0.6f}   {PFA_i:0.6f}   {PM_i:0.6f}   {Pe_i:0.6f}")\n',
            '\n',
            'plt.figure(figsize=(8, 4))\n',
            'plt.plot(pi1_values, gamma_map_values, marker="o", linestyle="-", label=r"$\gamma_{MAP}$")\n',
            'plt.xlabel(r"Prior $\pi_1$")\n',
            'plt.ylabel(r"Umbral $\gamma_{MAP}$")\n',
            'plt.title(r"Umbral MAP vs. prior $\pi_1$")\n',
            'plt.grid(True)\n',
            'plt.legend()\n',
            'plt.show()\n',
            '\n',
            'plt.figure(figsize=(8, 4))\n',
            'plt.plot(pi1_values, PFA_values, marker="o", label=r"$P_{FA}$")\n',
            'plt.plot(pi1_values, PM_values, marker="s", label=r"$P_{M}$")\n',
            'plt.plot(pi1_values, Pe_values, marker="^", label=r"$P_{e}$")\n',
            'plt.xlabel(r"Prior $\pi_1$")\n',
            'plt.ylabel("Probabilidad")\n',
            'plt.title("Probabilidades de error vs. prior")\n',
            'plt.grid(True)\n',
            'plt.legend()\n',
            'plt.show()\n',
        ],
    })

# Update the final markdown cell to explain the second point more clearly
if data['cells'] and data['cells'][-1]['cell_type'] == 'markdown':
    data['cells'][-1]['source'] = [
        '## Punto 2 - BinDetection_Lab (1).pdf\n',
        '\n',
        'En esta sección desarrollamos el segundo enunciado del laboratorio con los valores de prior probability indicados.\n',
        '\n',
        'Los pasos son:\n',
        '- Calcular el umbral $\\gamma_{MAP}$ para cada $\\pi_1 \\in \\{0.1,0.3,0.5,0.7,0.9\\}$.\n',
        '- Calcular $P_{FA}$, $P_M$ y $P_e$ en ese umbral.\n',
        '- Graficar $\\gamma_{MAP}$ vs. $\\pi_1$.\n',
        '- Graficar $P_{FA}$, $P_M$ y $P_e$ vs. $\\pi_1$.\n',
        '\n',
        'La expresión analítica de $\\gamma_{MAP}$ muestra que el umbral se desplaza hacia la hipótesis menos probable porque el término\n',
        '$\\ln\\left(\\frac{\\pi_0}{\\pi_1}\\right)$ cambia de signo según cuál hipótesis es más probable. Para ruido alto, este desplazamiento puede ser lo suficientemente grande como para mover el umbral fuera del intervalo $[a_0, a_1]$.\n',
    ]

with path.open('w', encoding='utf-8') as f:
    json.dump(data, f, ensure_ascii=False, indent=2)
