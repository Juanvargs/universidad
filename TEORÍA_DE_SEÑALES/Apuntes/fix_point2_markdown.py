import json
from pathlib import Path

path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')
with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

updated = False
for cell in data['cells']:
    if cell['cell_type'] == 'markdown' and any('## 5. Punto 2' in line or 'En este punto usamos' in line for line in cell.get('source', [])):
        cell['source'] = [
            '## 5. Punto 2: efecto de las probabilidades a priori\n',
            '\n',
            'En este punto usamos la misma configuración de detección binaria con ruido AWGN, pero variamos las probabilidades a priori de la hipótesis $H_1$.\n',
            'Para cada valor de $\pi_1 \in \{0.1,0.3,0.5,0.7,0.9\}$ calculamos el umbral MAP y las probabilidades de error correspondientes.\n',
            '\n',
            'La expresión analítica del umbral MAP es:\n',
            '$$\gamma_{MAP} = \frac{a_0 + a_1}{2} + \frac{\sigma_{\eta}^2}{a_1 - a_0} \ln\left(\frac{\pi_0}{\pi_1}\right)$$\n',
            '\n',
            'Este umbral se desplaza hacia la hipótesis menos probable porque el término logarítmico cambia de signo según cuál prior sea mayor.\n',
            'Con priors muy desbalanceadas y ruido suficiente, el umbral puede incluso salir del intervalo $[a_0, a_1]$.\n',
            '\n',
            'A continuación calculamos y graficamos:\n',
            '- $\gamma_{MAP}$ en función de $\pi_1$.\n',
            '- $P_{FA}$, $P_M$ y $P_e$ en el umbral MAP.\n',
        ]
        updated = True
        break

if not updated:
    raise SystemExit('No markdown cell found to update')

with path.open('w', encoding='utf-8') as f:
    json.dump(data, f, ensure_ascii=False, indent=2)
print('Updated Punto 2 markdown explanation.')
