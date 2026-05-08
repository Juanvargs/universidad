# Repaso parcial 1

## 1. Señales aleatorias y estadística básica

### 1. Idea principal

Una **señal aleatoria** es una señal cuyo valor no se puede predecir exactamente antes de observarla. En vez de describirla con una única función determinista, se describe mediante un **proceso aleatorio**, es decir, una familia de posibles señales.

- **Proceso aleatorio:** regla que asigna una función temporal a cada resultado de un experimento aleatorio.
- **Señal aleatoria continua:** proceso definido para tiempo continuo, por ejemplo \(x(t)\).
- **Señal aleatoria discreta:** proceso definido en instantes discretos, por ejemplo \(x[n]\).
- **Realización:** una señal concreta obtenida al repetir el experimento una vez.
- **Observación:** el valor medido de una realización en un intervalo o instante.
- **Ensamble:** conjunto de todas las realizaciones posibles.
- **Promedio de ensamble:** promedio entre realizaciones en un mismo instante.
- **Promedio temporal:** promedio a lo largo del tiempo de una realización.

El objetivo es reemplazar la descripción exacta, que no se conoce, por estadísticas: media, varianza, momentos, RMS y potencia.

Un **proceso de segundo orden** es un proceso para el cual existen y son finitos los momentos de orden 1 y 2, especialmente \(E\{x(t)\}\) y \(E\{|x(t)|^2\}\). Esto permite hablar de media, potencia, varianza y autocorrelación.

### 2. Fórmulas principales

Media o esperanza:

$$
\mu_x(t)=E\{x(t)\}
$$

Momento de orden \(n\):

$$
m_n(t)=E\{x^n(t)\}
$$

Varianza:

$$
\sigma_x^2(t)=E\{(x(t)-\mu_x(t))^2\}
$$

Potencia promedio:

$$
P_x=E\{|x(t)|^2\}
$$

Valor RMS:

$$
x_{RMS}=\sqrt{E\{x^2(t)\}}
$$

Relación útil para señales reales:

$$
P_x=E\{x^2(t)\}=\sigma_x^2(t)+\mu_x^2(t)
$$

Componente DC:

$$
x_{DC}=\mu_x
$$

Componente alternante:

$$
x_{AC}(t)=x(t)-\mu_x
$$

### 3. Significado de las variables

- \(x(t)\): señal aleatoria en tiempo continuo.
- \(x[n]\): señal aleatoria en tiempo discreto.
- \(E\{\cdot\}\): esperanza matemática, promedio estadístico sobre el ensamble.
- \(\mu_x(t)\): media del proceso en el instante \(t\).
- \(m_n(t)\): momento de orden \(n\).
- \(\sigma_x^2(t)\): varianza.
- \(\sigma_x(t)\): desviación estándar, raíz de la varianza.
- \(P_x\): potencia promedio estadística.
- \(x_{RMS}\): valor eficaz o raíz cuadrática media.
- \(x_{DC}\): componente constante.
- \(x_{AC}(t)\): fluctuación alrededor de la media.

### 4. ¿Por qué se hace esto?

Cuando una señal es aleatoria, no basta con decir cuánto vale en un instante. Se necesitan cantidades que resuman su comportamiento:

- La **media** dice alrededor de qué nivel se concentra la señal.
- La **varianza** mide cuánto se dispersa alrededor de la media.
- El **RMS** mide el valor eficaz de la señal.
- La **potencia** mide energía promedio por unidad de tiempo, o intensidad cuadrática promedio.

Se usan esperanzas porque el valor exacto cambia entre realizaciones, pero sus estadísticas pueden ser estables y útiles para diseño, análisis y medición.

### 5. Interpretación física

La **media** es el nivel DC de la señal. Si una señal eléctrica tiene media positiva, tiene un desplazamiento constante hacia arriba. Si tiene media cero, no tiene componente DC.

La **varianza** mide la potencia de la parte alternante cuando la señal tiene media constante:

$$
\sigma_x^2=E\{(x-\mu_x)^2\}
$$

La **potencia** incluye tanto la parte DC como la parte alternante:

$$
P_x=\mu_x^2+\sigma_x^2
$$

El **RMS** es la raíz de la potencia para señales reales:

$$
x_{RMS}=\sqrt{P_x}
$$

Por eso no son lo mismo:

| Cantidad | Qué mide | Incluye DC | Unidades |
|---|---:|---:|---|
| Media | Nivel promedio | Sí | Igual a \(x\) |
| Varianza | Dispersión respecto a la media | No | \(x^2\) |
| RMS | Valor eficaz | Sí | Igual a \(x\) |
| Potencia | Promedio cuadrático | Sí | \(x^2\) |

### 6. Recuadro importante

> [!IMPORTANT]
> Para una señal real de media constante: \(P_x=E\{x^2\}=\sigma_x^2+\mu_x^2\). Si \(\mu_x=0\), entonces potencia y varianza coinciden.
> Memoriza: media, varianza, potencia, RMS y la relación \(P_x=\sigma_x^2+\mu_x^2\).
> Debes saber demostrar: que la potencia se separa en componente DC más componente alternante.

> [!NOTE]
> La media describe el centro de la señal; la varianza describe cuánto se mueve alrededor de ese centro; el RMS describe el tamaño eficaz total.

> [!WARNING]
> No confundas RMS con media. Una señal senoidal pura de media cero tiene RMS distinto de cero.

> [!TIP]
> En parcial, si te piden potencia de una señal aleatoria, busca \(E\{|x(t)|^2\}\), no \(E\{x(t)\}\).

### 7. Preguntas tipo parcial

1. Dado un proceso aleatorio, calcule \(\mu_x(t)\), \(\sigma_x^2(t)\), \(x_{RMS}\) y \(P_x\).
2. Explique la diferencia entre realización, observación y ensamble.
3. Demuestre que \(P_x=\sigma_x^2+\mu_x^2\) para una señal real.
4. Interprete físicamente la media y la varianza de una señal de ruido.

### 8. Errores comunes

1. Creer que una realización es todo el proceso aleatorio.
2. Calcular la potencia como \(E\{x(t)\}\) en lugar de \(E\{|x(t)|^2\}\).
3. Olvidar que la varianza mide dispersión respecto a la media, no respecto a cero.
4. Decir que RMS y desviación estándar siempre son iguales; solo coinciden si la media es cero.

### 9. Mini-ejemplo

Sea \(x(t)=A+n(t)\), donde \(A\) es constante y \(n(t)\) tiene media cero y varianza \(\sigma_n^2\).

Media:

$$
\mu_x=E\{A+n(t)\}=A
$$

Varianza:

$$
\sigma_x^2=E\{(x(t)-A)^2\}=E\{n^2(t)\}=\sigma_n^2
$$

Potencia:

$$
P_x=A^2+\sigma_n^2
$$

Interpretación: \(A^2\) es potencia DC y \(\sigma_n^2\) es potencia fluctuante.

Ejercicios guía:

1. Calcule media y RMS de \(x(t)=3+n(t)\), con \(\sigma_n^2=4\).
2. Determine cuándo \(P_x=\sigma_x^2\).
3. Explique qué representa una media negativa en una señal eléctrica.

---

## 2. Estacionariedad y ergodicidad

### 1. Idea principal

La **estacionariedad** indica que las estadísticas de un proceso no cambian al desplazar el origen temporal. En cursos de señales se usa mucho la **estacionariedad en sentido amplio**, o **WSS** por sus siglas en inglés.

Un proceso es **WSS** si cumple dos condiciones:

1. Su media es constante.
2. Su autocorrelación depende solo del retardo \(\tau=t_2-t_1\), no de los tiempos absolutos.

La **ergodicidad** indica que las estadísticas del ensamble pueden obtenerse a partir de una sola realización suficientemente larga. Esto es clave cuando se trabaja con datos medidos, porque en laboratorio casi nunca se tiene acceso a infinitas realizaciones.

### 2. Fórmulas principales

Media constante:

$$
E\{x(t)\}=\mu_x=\text{constante}
$$

Autocorrelación WSS:

$$
R_x(t_1,t_2)=R_x(t_2-t_1)=R_x(\tau)
$$

Media temporal para un proceso ergódico en media:

$$
\mu_x=
\lim_{T\to\infty}
\frac{1}{T}
\int_0^T x(t)\,dt
$$

Autocorrelación temporal para un proceso ergódico en autocorrelación:

$$
R_x(\tau)=
\lim_{T\to\infty}
\frac{1}{T}
\int_0^T x(t)x^*(t+\tau)\,dt
$$

### 3. Significado de las variables

- \(t_1,t_2\): instantes absolutos.
- \(\tau=t_2-t_1\): retardo temporal.
- \(\mu_x\): media constante.
- \(R_x(t_1,t_2)\): autocorrelación general.
- \(R_x(\tau)\): autocorrelación para proceso WSS.
- \(T\): duración de observación.
- \(^*\): conjugado complejo.

### 4. ¿Por qué se hace esto?

La estacionariedad simplifica el análisis. Si la autocorrelación dependiera de \(t_1\) y \(t_2\) por separado, estudiar el proceso sería mucho más difícil. Cuando depende solo de \(\tau\), se puede hablar de memoria temporal, potencia y PSD de forma directa.

La ergodicidad permite reemplazar promedios de ensamble por promedios temporales. Eso justifica estimar media, autocorrelación y PSD usando una única señal medida.

### 5. Interpretación física

Un proceso WSS tiene comportamiento estadístico estable: su nivel promedio y su estructura de correlación no cambian con el tiempo. Por ejemplo, ruido térmico ideal en régimen permanente suele modelarse como WSS.

Un proceso ergódico es más fuerte: una realización larga contiene suficiente información estadística del proceso completo.

WSS no significa necesariamente ergódico. Ejemplo conceptual: si cada realización es una constante aleatoria \(X(t)=A\), con \(A\) distinto para cada realización, el proceso puede ser estacionario porque nada cambia con el tiempo, pero una sola realización solo revela un valor de \(A\), no toda la distribución del ensamble.

Todo proceso ergódico debe ser estacionario en la estadística considerada, porque si el promedio temporal representa al ensamble, esas estadísticas no pueden depender del origen temporal. Pero no todo proceso estacionario es ergódico.

### 6. Recuadro importante

> [!IMPORTANT]
> WSS significa media constante y autocorrelación dependiente solo de \(\tau\). Ergodicidad significa que puedo estimar estadísticas del ensamble usando una sola realización larga.
> Memoriza: las dos condiciones WSS y la diferencia entre promedio temporal y promedio de ensamble.
> Debes saber demostrar: si un proceso es WSS calculando media y autocorrelación.

> [!NOTE]
> En mediciones reales, casi siempre asumimos ergodicidad para justificar que una captura temporal representa al proceso.

> [!WARNING]
> Que un proceso sea estacionario no garantiza que una sola realización baste para estimar sus estadísticas.

> [!TIP]
> Para probar WSS, revisa en orden: media constante y autocorrelación dependiente solo del retardo.

### 7. Preguntas tipo parcial

1. Determine si un proceso dado es WSS.
2. Explique por qué WSS no implica ergodicidad.
3. Explique cuándo se puede usar una sola realización para estimar la media.
4. Dé un ejemplo de proceso estacionario no ergódico.

### 8. Errores comunes

1. Decir que estacionario significa que la señal no cambia en el tiempo.
2. Confundir promedio temporal con promedio de ensamble.
3. Suponer que WSS y ergodicidad son equivalentes.
4. Usar fórmulas de PSD para procesos no WSS sin justificarlo.

### 9. Mini-ejemplo

Sea \(x(t)=A\cos(\omega_0t+\phi)\), con \(\phi\) uniforme en \([0,2\pi)\).

Media:

$$
E\{x(t)\}=0
$$

Autocorrelación:

$$
R_x(t_1,t_2)=\frac{A^2}{2}\cos(\omega_0(t_2-t_1))
$$

Como depende solo de \(\tau\), el proceso es WSS.

Ejercicios guía:

1. Revise si \(x(t)=A\cos(\omega_0t)\) con \(A\) aleatoria es WSS.
2. Explique qué falla si la media depende de \(t\).
3. Compare \(E\{x(t)\}\) con \(\frac{1}{T}\int_0^T x(t)dt\).

---

## 3. Autocorrelación, covarianza y correlación cruzada

### 1. Idea principal

La **autocorrelación** mide qué tanto se parece una señal a una versión retardada de sí misma. Sirve para medir memoria temporal, periodicidad, potencia y estructura interna de un proceso aleatorio.

La **covarianza** mide la correlación después de quitar la media. Por eso representa la relación entre las partes fluctuantes.

La **correlación cruzada** mide qué tanto se parecen dos señales diferentes cuando una se desplaza respecto a la otra.

### 2. Fórmulas principales

Autocorrelación general:

$$
R_x(t_1,t_2)=E\{x(t_1)x^*(t_2)\}
$$

Autocorrelación WSS:

$$
R_x(\tau)=E\{x(t)x^*(t+\tau)\}
$$

Covarianza:

$$
K_x(\tau)=E\{(x(t)-\mu_x)(x^*(t+\tau)-\mu_x^*)\}
$$

Relación entre covarianza y autocorrelación:

$$
K_x(\tau)=R_x(\tau)-|\mu_x|^2
$$

Correlación cruzada:

$$
R_{xy}(\tau)=E\{x(t)y^*(t+\tau)\}
$$

Potencia:

$$
R_x(0)=E\{|x(t)|^2\}=P_x
$$

Varianza:

$$
K_x(0)=\sigma_x^2
$$

Máximo en cero:

$$
|R_x(\tau)|\leq R_x(0)
$$

Simetría conjugada:

$$
R_x(\tau)=R_x^*(-\tau)
$$

Para señales reales:

$$
R_x(\tau)=R_x(-\tau)
$$

Diferencia cuadrática media:

$$
\varepsilon_x(\tau)=E\{(x(t)-x(t+\tau))^2\}
$$

Para señales reales WSS:

$$
\varepsilon_x(\tau)=2R_x(0)-2R_x(\tau)
$$

Intervalo de correlación:

$$
\frac{R_x(\tau_c)}{\sigma_x^2}=\beta
$$

con:

$$
\beta \approx 0.05 \text{ a } 0.1
$$

Tiempo efectivo de correlación:

$$
\Delta \tau_e=
\frac{1}{R_x(0)}
\int_0^\infty R_x(\tau)\,d\tau
$$

Ejemplos típicos:

Ruido blanco:

$$
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
$$

Seno con fase aleatoria:

$$
x(t)=a\cos(\omega_0t+\phi)
$$

$$
R_x(\tau)=\frac{a^2}{2}\cos(\omega_0\tau)
$$

ACF exponencial:

$$
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
$$

### 3. Significado de las variables

- \(R_x(\tau)\): autocorrelación del proceso \(x(t)\).
- \(R_{xy}(\tau)\): correlación cruzada entre \(x(t)\) y \(y(t)\).
- \(K_x(\tau)\): covarianza.
- \(\mu_x\): media del proceso.
- \(\sigma_x^2\): varianza.
- \(\tau\): retardo.
- \(\tau_c\): intervalo de correlación.
- \(\Delta\tau_e\): tiempo efectivo de correlación.
- \(\beta\): umbral pequeño para decidir cuándo la correlación es despreciable.
- \(\delta(\tau)\): delta de Dirac.
- \(N_0/2\): nivel de PSD bilateral del ruido blanco.

### 4. ¿Por qué se hace esto?

La autocorrelación se usa porque no solo importa cuánta potencia tiene una señal, sino cómo está organizada en el tiempo. Dos señales pueden tener la misma potencia pero memorias temporales distintas.

Si \(R_x(\tau)\) decae rápido, valores separados en el tiempo se vuelven casi independientes rápidamente. Si \(R_x(\tau)\) decae lento, la señal conserva memoria. Si \(R_x(\tau)\) es periódica, la señal contiene estructura periódica.

La covarianza se usa cuando interesa quitar la componente DC y estudiar solo las fluctuaciones.

### 5. Interpretación física

Que una señal tenga **memoria** significa que conocer \(x(t)\) ayuda a predecir \(x(t+\tau)\). La memoria se mide con \(R_x(\tau)\).

Si la autocorrelación decae rápido, la señal cambia de forma brusca o irregular; esto suele asociarse con contenido espectral ancho.

Si la autocorrelación es periódica, la señal repite patrones; esto suele asociarse con líneas espectrales o componentes sinusoidales.

\(R_x(0)\) representa potencia porque en \(\tau=0\):

$$
R_x(0)=E\{x(t)x^*(t)\}=E\{|x(t)|^2\}
$$

Para la ACF exponencial, si \(\alpha\) aumenta, \(e^{-\alpha|\tau|}\) decae más rápido. Eso significa menor tiempo de correlación y menor memoria temporal.

### 6. Recuadro importante

> [!IMPORTANT]
> La autocorrelación en \(\tau=0\) da la potencia total. La covarianza en \(\tau=0\) da la varianza.
> Memoriza: \(R_x(0)=P_x\), \(K_x(0)=\sigma_x^2\), simetría y máximo en cero.
> Debes saber demostrar: la ACF del seno con fase aleatoria y la relación de diferencia cuadrática media.

> [!NOTE]
> Autocorrelación ancha significa señal con memoria larga. Autocorrelación angosta significa señal menos predecible en el tiempo.

> [!WARNING]
> No confundas autocorrelación con covarianza: la autocorrelación incluye la media; la covarianza no.

> [!TIP]
> Si la señal es real y WSS, \(R_x(\tau)\) debe ser par. Si obtienes una ACF no par, revisa el cálculo.

### 7. Preguntas tipo parcial

1. Calcule \(R_x(\tau)\) para una señal senoidal con fase aleatoria.
2. Demuestre que \(R_x(0)=P_x\).
3. Explique la diferencia entre autocorrelación y covarianza.
4. Interprete una ACF exponencial cuando \(\alpha\) aumenta.

### 8. Errores comunes

1. Pensar que \(R_x(\tau)\) siempre tiende a cero; si hay media no nula puede tender a \(|\mu_x|^2\).
2. Olvidar el conjugado en señales complejas.
3. Confundir el retardo \(\tau\) con el tiempo absoluto \(t\).
4. Usar \(K_x(\tau)=R_x(\tau)\) aunque la media no sea cero.

### 9. Mini-ejemplo

Para \(R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}\):

Potencia:

$$
R_x(0)=\sigma_x^2
$$

Tiempo efectivo:

$$
\Delta\tau_e=\frac{1}{\sigma_x^2}\int_0^\infty \sigma_x^2e^{-\alpha\tau}d\tau=\frac{1}{\alpha}
$$

Interpretación: si \(\alpha\) se duplica, el tiempo de correlación se reduce a la mitad.

Ejercicios guía:

1. Calcule \(\varepsilon_x(\tau)\) para la ACF exponencial.
2. Determine \(K_x(\tau)\) si \(R_x(\tau)=4e^{-2|\tau|}+9\) y \(\mu_x=3\).
3. Explique si un ruido blanco tiene memoria temporal.

---

## 4. PSD, espectro estadístico y teorema de Wiener-Khinchine

### 1. Idea principal

La **densidad espectral de potencia**, o **PSD**, indica cómo se reparte la potencia de una señal aleatoria en frecuencia. Si una señal tiene mucha potencia alrededor de cierta frecuencia, la PSD toma valores altos allí.

Para procesos WSS, la autocorrelación y la PSD forman un par de Fourier. Esta relación se llama **teorema de Wiener-Khinchine**. Permite pasar del análisis temporal estadístico \(R_x(\tau)\) al análisis frecuencial estadístico \(S_x(\omega)\).

### 2. Fórmulas principales

Definición de PSD:

$$
S_x(\omega)=
\lim_{T\to\infty}
\frac{E\{|X_T(\omega)|^2\}}{2T}
$$

Potencia total:

$$
P_x=R_x(0)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
$$

Propiedades:

$$
S_x(\omega)\in \mathbb{R}
$$

$$
S_x(\omega)\geq 0
$$

Para procesos reales:

$$
S_x(\omega)=S_x(-\omega)
$$

Si la media es cero:

$$
\sigma_x^2=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
$$

Teorema de Wiener-Khinchine:

$$
S_x(\omega)=\mathcal{F}\{R_x(\tau)\}
$$

$$
R_x(\tau)=\mathcal{F}^{-1}\{S_x(\omega)\}
$$

Forma integral:

$$
S_x(\omega)=
\int_{-\infty}^{\infty}
R_x(\tau)e^{-j\omega\tau}\,d\tau
$$

$$
R_x(\tau)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}
S_x(\omega)e^{j\omega\tau}\,d\omega
$$

Para funciones pares:

$$
S_x(\omega)=
2\int_0^\infty R_x(\tau)\cos(\omega\tau)\,d\tau
$$

$$
R_x(\tau)=
\frac{1}{\pi}
\int_0^\infty S_x(\omega)\cos(\omega\tau)\,d\omega
$$

Pares ACF-PSD frecuentes:

ACF exponencial:

$$
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
$$

$$
S_x(\omega)=
\frac{2\sigma_x^2\alpha}{\alpha^2+\omega^2}
$$

Ruido blanco:

$$
S_n(\omega)=\frac{N_0}{2}
$$

$$
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
$$

PSD rectangular:

$$
S_n(\omega)=
\begin{cases}
\frac{N_0}{2}, & |\omega|<B\\
0, & \text{otro caso}
\end{cases}
$$

$$
R_n(\tau)=
\frac{N_0B}{2\pi}
\operatorname{sinc}
\left(
\frac{B\tau}{\pi}
\right)
$$

Seno con fase aleatoria:

$$
R_x(\tau)=
\frac{a^2}{2}\cos(\omega_0\tau)
$$

$$
S_x(\omega)=
\frac{\pi a^2}{2}
[
\delta(\omega-\omega_0)+\delta(\omega+\omega_0)
]
$$

### 3. Significado de las variables

- \(S_x(\omega)\): densidad espectral de potencia.
- \(X_T(\omega)\): transformada de Fourier de una observación truncada de duración \(2T\).
- \(\omega\): frecuencia angular en rad/s.
- \(P_x\): potencia promedio total.
- \(R_x(\tau)\): autocorrelación.
- \(B\): frecuencia angular de corte en la PSD rectangular.
- \(\alpha\): parámetro de decaimiento de la ACF exponencial.
- \(\operatorname{sinc}(u)\): función sinc normalizada, \(\operatorname{sinc}(u)=\frac{\sin(\pi u)}{\pi u}\).

### 4. ¿Por qué se hace esto?

La PSD se usa porque muchas operaciones de señales son más claras en frecuencia. Por ejemplo, un filtro elimina o atenúa bandas de frecuencia; la PSD permite ver cuánta potencia se pierde o se conserva.

Wiener-Khinchine se usa porque la autocorrelación contiene información de potencia y memoria temporal, mientras que la PSD contiene la misma información organizada por frecuencia.

La PSD no puede ser negativa porque viene de un promedio de magnitudes cuadradas:

$$
E\{|X_T(\omega)|^2\}\geq 0
$$

La PSD se integra para obtener potencia porque es una densidad: cada intervalo de frecuencia aporta una parte de la potencia total.

### 5. Interpretación física

Una autocorrelación angosta implica un espectro ancho. Si la señal pierde memoria muy rápido, cambia bruscamente en el tiempo y necesita muchas frecuencias para describirse.

Una autocorrelación ancha implica un espectro angosto. Si la señal cambia lentamente y conserva memoria, su potencia se concentra en frecuencias bajas o en pocas frecuencias.

El ruido blanco ideal tiene PSD plana: todas las frecuencias tienen la misma densidad de potencia. Por eso su autocorrelación es una delta: no hay memoria entre instantes distintos.

### 6. Recuadro importante

> [!IMPORTANT]
> Para procesos WSS: autocorrelación y PSD son par de Fourier. Además, el área bajo \(S_x(\omega)\), dividida por \(2\pi\), es la potencia total.
> Memoriza: Wiener-Khinchine, propiedades de la PSD y pares ACF-PSD frecuentes.
> Debes saber demostrar: PSD de la ACF exponencial y ACF de la PSD rectangular.

> [!NOTE]
> Una línea delta en PSD representa potencia concentrada en una frecuencia exacta, como una sinusoide ideal.

> [!WARNING]
> La PSD no es la transformada de Fourier de la señal aleatoria directamente; es la transformada de Fourier de la autocorrelación.

> [!TIP]
> Si \(R_x(\tau)\) es par, usa las integrales con coseno para ahorrar trabajo.

### 7. Preguntas tipo parcial

1. Obtenga la PSD de una ACF exponencial.
2. Obtenga la ACF de una PSD rectangular.
3. Explique por qué una PSD debe ser real y no negativa.
4. Interprete físicamente una PSD con deltas en \(\pm\omega_0\).

### 8. Errores comunes

1. Olvidar el factor \(1/(2\pi)\) en la transformada inversa.
2. Confundir frecuencia angular \(\omega\) con frecuencia en Hz \(f\).
3. Interpretar la PSD como amplitud en vez de potencia por frecuencia.
4. Pensar que una PSD plana ideal tiene potencia finita en ancho de banda infinito.

### 9. Mini-ejemplo

Dada:

$$
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
$$

Como es par:

$$
S_x(\omega)=2\int_0^\infty \sigma_x^2e^{-\alpha\tau}\cos(\omega\tau)d\tau
$$

Usando:

$$
\int_0^\infty e^{-\alpha\tau}\cos(\omega\tau)d\tau=\frac{\alpha}{\alpha^2+\omega^2}
$$

Entonces:

$$
S_x(\omega)=\frac{2\sigma_x^2\alpha}{\alpha^2+\omega^2}
$$

Ejercicios guía:

1. Integre esta PSD y verifique que \(P_x=\sigma_x^2\).
2. Explique qué pasa con el espectro si \(\alpha\) aumenta.
3. Dibuje cualitativamente la PSD de una sinusoide aleatoria.

---

## 5. Estimadores de ACF y PSD

### 1. Idea principal

Las funciones teóricas \(R_x(\tau)\) y \(S_x(\omega)\) suponen promedios infinitos o ensambles ideales. En la práctica solo tenemos una secuencia finita de datos. Por eso usamos **estimadores**.

La **autocorrelación muestral** aproxima la ACF usando muestras. El **periodograma** aproxima la PSD usando la magnitud cuadrada de la transformada discreta. El **método de Welch** mejora el periodograma dividiendo la señal en segmentos, aplicando ventanas y promediando.

### 2. Fórmulas principales

Estimador sesgado:

$$
\hat{R}_{x,\text{sesgada}}[k]
=
\frac{1}{N}
\sum_{n=0}^{N-1-k}
x[n]x^*[n+k]
$$

Estimador no sesgado:

$$
\hat{R}_{x,\text{no sesgada}}[k]
=
\frac{1}{N-k}
\sum_{n=0}^{N-1-k}
x[n]x^*[n+k]
$$

Transformada discreta:

$$
X_N(e^{j\omega})=
\sum_{n=0}^{N-1}
x[n]e^{-j\omega n}
$$

Periodograma:

$$
\hat{S}_x(e^{j\omega})
=
\frac{1}{N}
|X_N(e^{j\omega})|^2
$$

Resolución espectral aproximada:

$$
\Delta f \approx \frac{f_s}{N}
$$

### 3. Significado de las variables

- \(x[n]\): secuencia medida.
- \(N\): número de muestras.
- \(k\): retardo discreto.
- \(\hat{R}_x[k]\): estimación de autocorrelación.
- \(X_N(e^{j\omega})\): transformada de la secuencia finita.
- \(\hat{S}_x(e^{j\omega})\): PSD estimada.
- \(f_s\): frecuencia de muestreo.
- \(\Delta f\): separación entre bins de frecuencia.

### 4. ¿Por qué se hace esto?

El estimador sesgado divide por \(N\) para todos los retardos. Esto produce menor varianza y suele dar matrices de autocorrelación mejor comportadas.

El estimador no sesgado divide por \(N-k\), porque para retardos grandes hay menos productos disponibles. Corrige el promedio, pero su varianza aumenta en retardos grandes.

El periodograma usa la idea de que la potencia en frecuencia está relacionada con \(|X_N|^2\). Sin embargo, tiene alta varianza: aunque aumente \(N\), el periodograma individual sigue siendo muy irregular.

Welch reduce la varianza al promediar varios periodogramas de segmentos. Se pierde algo de resolución, pero se gana estabilidad.

### 5. Interpretación física

Las estimaciones no son iguales a las funciones teóricas por varias razones:

- La señal medida tiene longitud finita.
- Puede haber ruido adicional.
- La ventana modifica el espectro.
- El muestreo limita la resolución.
- El periodograma tiene alta varianza.

Aumentar \(N\) mejora la resolución espectral y permite estimar retardos más largos, pero no elimina por completo la variabilidad del periodograma.

Las ventanas reducen fuga espectral, pero ensanchan los lóbulos principales. Por eso mejoran la lectura de picos cercanos a componentes fuertes, a costa de resolución.

Un pico espectral indica potencia concentrada cerca de una frecuencia. Un piso espectral indica ruido distribuido en muchas frecuencias.

### 6. Recuadro importante

> [!IMPORTANT]
> El periodograma es fácil de calcular con FFT, pero tiene alta varianza. Welch reduce la varianza promediando segmentos.
> Memoriza: estimador sesgado, estimador no sesgado, periodograma y pasos de Welch.
> Debes saber explicar: por qué una estimación finita no coincide exactamente con la función teórica.

> [!NOTE]
> Más muestras dan mejor resolución en frecuencia, pero no garantizan una PSD perfectamente suave.

> [!WARNING]
> No compares una PSD teórica ideal con una estimada esperando igualdad exacta punto a punto.

> [!TIP]
> En parcial, si preguntan por Welch: segmentar, ventanear, calcular periodogramas y promediar.

### 7. Preguntas tipo parcial

1. Compare el estimador sesgado y el no sesgado de autocorrelación.
2. Explique por qué el periodograma tiene alta varianza.
3. Explique cómo Welch reduce la varianza de la PSD estimada.
4. Analice el efecto de aumentar \(N\) sobre la resolución espectral.

### 8. Errores comunes

1. Creer que el estimador no sesgado siempre es mejor.
2. Olvidar que para retardos grandes hay menos productos disponibles.
3. Confundir fuga espectral con ruido.
4. Pensar que usar FFT cambia la definición matemática de la PSD; solo acelera el cálculo.

### 9. Mini-ejemplo

Si se tienen \(N=1000\) muestras y \(k=100\), el estimador no sesgado divide por \(900\), porque solo existen \(900\) productos \(x[n]x^*[n+100]\). El estimador sesgado divide por \(1000\), reduciendo el valor estimado en retardos grandes.

Ejercicios guía:

1. Calcule \(\hat{R}_x[1]\) para una secuencia corta real.
2. Explique qué pasa con el periodograma si se agrega ruido blanco.
3. Compare una ventana rectangular y una ventana Hann.

---

## 6. Ruido blanco, ruido gaussiano y ruido limitado en banda

### 1. Idea principal

El **ruido blanco** es un modelo de ruido cuya PSD es constante en todas las frecuencias. La palabra "blanco" se refiere al espectro, no a la distribución de amplitudes.

El **ruido gaussiano** es un ruido cuyas amplitudes siguen una distribución normal. La palabra "gaussiano" se refiere a la estadística de amplitud, no al espectro.

El **AWGN** es ruido blanco gaussiano aditivo: se suma a la señal, tiene PSD plana y sus amplitudes son gaussianas.

El ruido blanco ideal no es físicamente realizable porque tiene ancho de banda infinito y, por tanto, potencia infinita. En la práctica se usa ruido blanco limitado por el ancho de banda del sistema.

### 2. Fórmulas principales

Ruido blanco ideal:

$$
S_n(\omega)=\frac{N_0}{2}, \quad -\infty<\omega<\infty
$$

$$
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
$$

Ruido limitado en banda:

$$
S_n(\omega)=
\begin{cases}
\frac{N_0}{2}, & |\omega|<\Delta\omega\\
0, & |\omega|\geq \Delta\omega
\end{cases}
$$

$$
R_n(\tau)=
\frac{N_0\Delta\omega}{2\pi}
\operatorname{sinc}
\left(
\frac{\Delta\omega\tau}{\pi}
\right)
$$

Potencia de ruido:

$$
P_n=R_n(0)=
\frac{1}{2\pi}
\int_{-\Delta\omega}^{\Delta\omega}
\frac{N_0}{2}\,d\omega
$$

$$
P_n=
\frac{N_0\Delta\omega}{2\pi}
$$

SNR lineal:

$$
SNR=\frac{P_s}{P_n}
$$

SNR en dB:

$$
SNR_{dB}=
10\log_{10}
\left(
\frac{P_s}{P_n}
\right)
$$

### 3. Significado de las variables

- \(n(t)\): ruido.
- \(S_n(\omega)\): PSD del ruido.
- \(R_n(\tau)\): autocorrelación del ruido.
- \(N_0/2\): densidad espectral bilateral.
- \(\Delta\omega\): ancho de banda angular de ruido.
- \(P_n\): potencia de ruido.
- \(P_s\): potencia de señal.
- \(SNR\): relación señal a ruido.

### 4. ¿Por qué se hace esto?

El ruido se modela para predecir cuánto degrada una señal. En frecuencia, el ruido blanco agrega un piso espectral. En tiempo, el ruido agrega fluctuaciones rápidas e impredecibles.

La potencia de ruido se obtiene integrando la PSD sobre el ancho de banda donde el sistema deja pasar ruido. Por eso reducir ancho de banda reduce potencia de ruido.

### 5. Interpretación física

Blanco significa "misma densidad de potencia en todas las frecuencias". Gaussiano significa "histograma de amplitudes con forma normal".

El ruido blanco ideal tiene ACF tipo delta porque no hay correlación entre instantes distintos. El ruido limitado en banda tiene ACF tipo sinc, porque limitar frecuencia introduce correlación temporal.

En la PSD, el ruido aparece como un piso. Si el piso sube, la SNR baja. Si la señal tiene picos pequeños sobre un piso alto, se vuelve difícil detectarla.

### 6. Recuadro importante

> [!IMPORTANT]
> Blanco describe el espectro; gaussiano describe la distribución de amplitudes; aditivo significa que el ruido se suma a la señal.
> Memoriza: \(S_n(\omega)=N_0/2\), \(R_n(\tau)=N_0\delta(\tau)/2\), \(P_n=N_0\Delta\omega/(2\pi)\) y SNR.
> Debes saber demostrar: la potencia de ruido limitado en banda integrando su PSD.

> [!NOTE]
> El ruido blanco ideal es una idealización útil. Todo sistema físico tiene ancho de banda finito.

> [!WARNING]
> No digas que todo ruido gaussiano es blanco. Puede ser gaussiano y tener PSD no plana.

> [!TIP]
> Para ruido limitado en banda, calcula potencia integrando la PSD solo en la banda que pasa.

### 7. Preguntas tipo parcial

1. Explique la diferencia entre ruido blanco y ruido gaussiano.
2. Calcule la potencia de ruido para una PSD rectangular.
3. Explique por qué el ruido blanco ideal no es físicamente realizable.
4. Interprete el efecto de aumentar \(\Delta\omega\) sobre \(P_n\).

### 8. Errores comunes

1. Confundir \(N_0/2\) con potencia total.
2. Olvidar el factor \(1/(2\pi)\) al integrar en \(\omega\).
3. Decir que la ACF del ruido limitado en banda es una delta.
4. Usar \(10\log_{10}\) para amplitudes en lugar de potencias.

### 9. Mini-ejemplo

Si \(N_0=2\) y \(\Delta\omega=100\pi\):

$$
P_n=\frac{N_0\Delta\omega}{2\pi}
=\frac{2(100\pi)}{2\pi}=100
$$

Si \(P_s=1000\):

$$
SNR=\frac{1000}{100}=10
$$

$$
SNR_{dB}=10\log_{10}(10)=10\text{ dB}
$$

Ejercicios guía:

1. Calcule \(P_n\) si se duplica el ancho de banda.
2. Explique cómo cambia \(R_n(\tau)\) al limitar el ruido.
3. Dibuje la PSD de señal más AWGN.

---

## 7. Transformación de señales aleatorias por sistemas LTI

### 1. Idea principal

Un sistema **LTI** es lineal e invariante en el tiempo. Se describe por su respuesta al impulso \(h(t)\) o por su respuesta en frecuencia \(H(\omega)\).

Si una señal aleatoria WSS entra a un sistema LTI estable, la salida también puede analizarse mediante autocorrelación y PSD. La regla más importante es:

$$
S_y(\omega)=|H(\omega)|^2S_x(\omega)
$$

Esto significa que el sistema modifica la potencia de entrada en cada frecuencia según la ganancia cuadrática del filtro.

### 2. Fórmulas principales

Convolución:

$$
y(t)=h(t)*x(t)
$$

Dominio de frecuencia:

$$
Y(\omega)=H(\omega)X(\omega)
$$

PSD de salida:

$$
S_y(\omega)=|H(\omega)|^2S_x(\omega)
$$

Potencia de salida:

$$
P_y=R_y(0)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}
S_y(\omega)\,d\omega
$$

Para ruido blanco de entrada:

$$
S_y(\omega)=|H(\omega)|^2\frac{N_0}{2}
$$

### 3. Significado de las variables

- \(x(t)\): entrada aleatoria.
- \(y(t)\): salida.
- \(h(t)\): respuesta al impulso.
- \(H(\omega)\): respuesta en frecuencia.
- \(S_x(\omega)\): PSD de entrada.
- \(S_y(\omega)\): PSD de salida.
- \(P_y\): potencia promedio de salida.

### 4. ¿Por qué se hace esto?

En señales deterministas, un sistema LTI multiplica el espectro por \(H(\omega)\). Pero la PSD mide potencia, no amplitud. Como la potencia depende de magnitud cuadrada, la PSD se multiplica por:

$$
|H(\omega)|^2
$$

No se usa \(H(\omega)\) directamente porque \(H(\omega)\) puede tener fase, y la PSD no describe fase: describe distribución de potencia.

### 5. Interpretación física

Un filtro pasa bajas aplicado a ruido blanco deja pasar solo bajas frecuencias. La PSD de salida ya no es plana infinita: queda recortada por \(|H(\omega)|^2\).

Filtrar reduce potencia de ruido porque elimina parte del área bajo la PSD. Si el ancho de banda de salida es menor, pasa menos potencia de ruido.

Después del filtrado, la autocorrelación cambia. Un ruido blanco filtrado deja de ser completamente no correlacionado: al limitar su banda, se vuelve más suave en el tiempo y adquiere correlación temporal.

### 6. Recuadro importante

> [!IMPORTANT]
> En un sistema LTI, la PSD de salida es la PSD de entrada multiplicada por \(|H(\omega)|^2\). Luego la potencia se obtiene integrando \(S_y(\omega)\).
> Memoriza: \(y=h*x\), \(Y=HX\), \(S_y=|H|^2S_x\).
> Debes saber demostrar: por qué se usa \(|H(\omega)|^2\) y cómo calcular potencia de salida.

> [!NOTE]
> Filtrar en frecuencia cambia la memoria temporal. Reducir ancho de banda normalmente aumenta el tiempo de correlación.

> [!WARNING]
> No multipliques la PSD por \(H(\omega)\). La PSD se multiplica por la magnitud cuadrada.

> [!TIP]
> Para calcular potencia de salida: primero encuentra \(S_y(\omega)\), luego integra.

### 7. Preguntas tipo parcial

1. Calcule \(S_y(\omega)\) dada \(S_x(\omega)\) y \(H(\omega)\).
2. Explique por qué aparece \(|H(\omega)|^2\).
3. Calcule la potencia de ruido a la salida de un filtro pasa bajas ideal.
4. Explique cómo cambia la autocorrelación de ruido blanco después de filtrarlo.

### 8. Errores comunes

1. Confundir respuesta en frecuencia con PSD.
2. Olvidar integrar la PSD para obtener potencia.
3. Ignorar el ancho de banda del filtro al calcular ruido.
4. Creer que el filtro solo cambia amplitud temporal y no estadísticas.

### 9. Mini-ejemplo

Ruido blanco con:

$$
S_n(\omega)=\frac{N_0}{2}
$$

pasa por un filtro pasa bajas ideal:

$$
H(\omega)=
\begin{cases}
1, & |\omega|<B\\
0, & |\omega|\geq B
\end{cases}
$$

Entonces:

$$
S_y(\omega)=
\begin{cases}
\frac{N_0}{2}, & |\omega|<B\\
0, & |\omega|\geq B
\end{cases}
$$

Potencia:

$$
P_y=\frac{1}{2\pi}\int_{-B}^{B}\frac{N_0}{2}d\omega
=\frac{N_0B}{2\pi}
$$

Ejercicios guía:

1. Repita el cálculo si \(|H(\omega)|=2\) en la banda.
2. Explique qué pasa si el filtro es pasa banda.
3. Relacione el ancho de banda de salida con \(R_y(\tau)\).

---

## 8. Modulación DSB, canal AWGN y demodulación

### 1. Idea principal

La **modulación DSB** traslada un mensaje de banda base alrededor de una frecuencia portadora \(\omega_c\). DSB significa **double sideband**, porque aparecen dos bandas laterales: una arriba y otra abajo de la portadora.

En **DSB-SC**, la portadora está suprimida. En **AM convencional** o **DSB-LC**, la portadora grande sí se transmite para permitir detección de envolvente.

Un **canal AWGN** agrega ruido blanco gaussiano aditivo a la señal modulada. La demodulación intenta recuperar el mensaje original desde la señal recibida.

### 2. Fórmulas principales

#### 8.1 DSB-SC

Señal modulada:

$$
s(t)=A_cm(t)\cos(\omega_ct)
$$

Espectro:

$$
S(\omega)=
\frac{A_c}{2}
[
M(\omega-\omega_c)+M(\omega+\omega_c)
]
$$

Ancho de banda:

$$
B_{DSB}=2B_m
$$

#### 8.2 AM convencional o DSB-LC

$$
s(t)=A_c[1+\mu m_n(t)]\cos(\omega_ct)
$$

Condición para evitar sobremodulación:

$$
0\leq\mu\leq 1
$$

Casos:

$$
\mu<1
$$

Submodulación.

$$
\mu=1
$$

Modulación al límite.

$$
\mu>1
$$

Sobremodulación.

#### 8.3 Canal AWGN

$$
y(t)=x(t)+n(t)
$$

$$
n(t)\sim AWGN
$$

Si señal y ruido no están correlacionados:

$$
S_y(\omega)=S_x(\omega)+S_n(\omega)
$$

#### 8.4 Demodulación coherente con filtro convencional

Partiendo de DSB-SC recibido ideal:

$$
y(t)\cos(\omega_ct)
=
A_cm(t)\cos^2(\omega_ct)
$$

Usando:

$$
\cos^2(\omega_ct)
=
\frac{1+\cos(2\omega_ct)}{2}
$$

Entonces:

$$
y(t)\cos(\omega_ct)
=
\frac{A_c}{2}m(t)
+
\frac{A_c}{2}m(t)\cos(2\omega_ct)
$$

Después del filtro pasa bajas:

$$
\hat{m}(t)=
\frac{A_c}{2}m(t)
$$

Con error de fase \(\theta\), la recuperación se escala aproximadamente por:

$$
\cos(\theta)
$$

#### 8.5 Demodulación con Hilbert

Señal analítica:

$$
x_a(t)=x(t)+j\mathcal{H}\{x(t)\}
$$

Envolvente:

$$
A_x(t)=|x_a(t)|
$$

Fase instantánea:

$$
\phi_x(t)=\arg\{x_a(t)\}
$$

### 3. Significado de las variables

- \(m(t)\): mensaje en banda base.
- \(m_n(t)\): mensaje normalizado, usualmente con \(|m_n(t)|\leq 1\).
- \(A_c\): amplitud de portadora.
- \(\omega_c\): frecuencia angular de portadora.
- \(B_m\): ancho de banda del mensaje.
- \(B_{DSB}\): ancho de banda de la señal DSB.
- \(\mu\): índice de modulación en AM.
- \(x(t)\): señal transmitida o señal antes del canal.
- \(y(t)\): señal recibida con ruido.
- \(n(t)\): ruido AWGN.
- \(\mathcal{H}\{\cdot\}\): transformada de Hilbert.
- \(x_a(t)\): señal analítica.
- \(A_x(t)\): envolvente.
- \(\phi_x(t)\): fase instantánea.

### 4. ¿Por qué se hace esto?

Multiplicar por una portadora \(\cos(\omega_ct)\) traslada el espectro porque:

$$
\cos(\omega_ct)=\frac{1}{2}(e^{j\omega_ct}+e^{-j\omega_ct})
$$

Multiplicar en tiempo equivale a desplazar en frecuencia. Por eso aparecen dos copias del mensaje: una centrada en \(+\omega_c\) y otra en \(-\omega_c\).

El ancho de banda DSB es \(2B_m\) porque la banda superior ocupa \(B_m\) por encima de la portadora y la inferior ocupa \(B_m\) por debajo.

En demodulación coherente se multiplica otra vez por la portadora para bajar el espectro a banda base. El producto genera una componente útil en baja frecuencia y otra alrededor de \(2\omega_c\), que se elimina con un filtro pasa bajas.

Hilbert se usa para construir la señal analítica, obtener envolvente y fase instantánea sin multiplicar explícitamente por una portadora sincronizada.

### 5. Interpretación física

En DSB-SC, la portadora no aparece como una línea dominante; solo se transmiten bandas laterales. Esto ahorra potencia, pero exige demodulación coherente.

En AM convencional o DSB-LC, se transmite una portadora grande. La envolvente es:

$$
A_c[1+\mu m_n(t)]
$$

Si \(\mu<1\), hay submodulación: la envolvente sigue al mensaje pero con margen. Si \(\mu=1\), la modulación llega al límite sin cruzar cero. Si \(\mu>1\), hay sobremodulación: la envolvente cruza cero y se invierte, produciendo distorsión en detectores de envolvente.

En un canal AWGN, en tiempo se observan fluctuaciones superpuestas. En PSD, el ruido agrega un piso espectral:

$$
S_y(\omega)=S_x(\omega)+S_n(\omega)
$$

En autocorrelación, si el ruido es independiente:

$$
R_y(\tau)=R_x(\tau)+R_n(\tau)
$$

SNR alto significa que la potencia de señal domina al ruido. SNR bajo significa que el ruido puede ocultar detalles del mensaje.

Hilbert es útil para analizar envolvente y fase, especialmente en señales AM y señales pasabanda. Puede fallar o volverse sensible cuando hay mucho ruido, sobremodulación, cruces por cero, componentes no separadas o señales que no cumplen bien la hipótesis de banda estrecha.

#### Comparación final

| Criterio | Filtro convencional | Hilbert |
|---|---|---|
| Calidad de recuperación | Muy buena si hay sincronización y buen filtrado | Buena si la señal analítica representa bien la señal |
| Sensibilidad al ruido | Depende del canal y del filtro pasa bajas | Alta en envolvente y fase instantánea |
| Necesidad de sincronización | Sí, requiere fase y frecuencia correctas | No siempre requiere portadora local explícita |
| Efecto del índice de modulación | En AM, sobremodulación distorsiona la envolvente | También sufre con cruces e inversión de envolvente |
| Distorsión | Aparece por error de fase, frecuencia o filtro mal elegido | Aparece por ruido, mala separación espectral o envolvente mal definida |
| Complejidad | Conceptualmente simple; requiere oscilador local y LPF | Requiere transformada de Hilbert y señal analítica |
| Interpretación de envolvente y fase | Indirecta | Directa mediante \(|x_a(t)|\) y \(\arg(x_a(t))\) |

### 6. Recuadro importante

> [!IMPORTANT]
> DSB traslada el mensaje a \(\pm\omega_c\). Demodular coherentemente vuelve a multiplicar por la portadora y usa un pasa bajas para recuperar banda base.
> Memoriza: DSB-SC, AM convencional, condición \(0\leq\mu\leq1\), canal AWGN y fórmulas de Hilbert.
> Debes saber demostrar: traslado espectral, demodulación coherente y efecto del término en \(2\omega_c\).

> [!NOTE]
> DSB-SC ahorra potencia al suprimir la portadora, pero exige sincronización. AM convencional transmite portadora para facilitar recuperación por envolvente.

> [!WARNING]
> En AM convencional, \(\mu>1\) produce sobremodulación y distorsiona la envolvente.

> [!TIP]
> Para demodulación coherente, siempre separa el término de banda base y el término en \(2\omega_c\).

### 7. Preguntas tipo parcial

1. Demuestre que multiplicar por \(\cos(\omega_ct)\) genera dos bandas laterales.
2. Explique por qué \(B_{DSB}=2B_m\).
3. Desarrolle la demodulación coherente de DSB-SC.
4. Compare AM convencional y DSB-SC en potencia, espectro y demodulación.
5. Explique el efecto de AWGN sobre tiempo, PSD y autocorrelación.

### 8. Errores comunes

1. Creer que DSB-SC contiene una portadora grande visible en el espectro.
2. Olvidar el factor \(1/2\) al multiplicar por cosenos.
3. Usar detector de envolvente en DSB-SC sin portadora.
4. Ignorar que la demodulación coherente requiere sincronización de fase y frecuencia.
5. Confundir envolvente con la señal modulada completa.

### 9. Mini-ejemplo

Para DSB-SC:

$$
s(t)=A_cm(t)\cos(\omega_ct)
$$

Demodulamos:

$$
s(t)\cos(\omega_ct)=A_cm(t)\cos^2(\omega_ct)
$$

$$
=\frac{A_c}{2}m(t)+\frac{A_c}{2}m(t)\cos(2\omega_ct)
$$

Un filtro pasa bajas elimina el término en \(2\omega_c\):

$$
\hat{m}(t)=\frac{A_c}{2}m(t)
$$

Si se quiere recuperar exactamente \(m(t)\), se compensa la ganancia multiplicando por \(2/A_c\).

Ejercicios guía:

1. Dibuje la PSD de DSB-SC para un mensaje limitado a \(B_m\).
2. Explique qué cambia si se agrega AWGN.
3. Compare la envolvente para \(\mu=0.5\), \(\mu=1\) y \(\mu=1.5\).

---

## 9. PM: banda angosta, DSB y banda ancha

### 1. Idea principal

La **modulación en fase** o **PM** cambia la fase instantánea de una portadora según el mensaje. A diferencia de AM, la amplitud ideal de la señal PM permanece constante.

Si la desviación de fase es pequeña, se obtiene **PM de banda angosta**, que se parece a una portadora más una señal DSB-SC en cuadratura. Si la desviación de fase es grande, se obtiene **PM de banda ancha**, con múltiples bandas laterales y ancho de banda teórico infinito.

### 2. Fórmulas principales

#### 9.1 Señal PM

Fórmula general:

$$
s_{PM}(t)=A_c\cos(\omega_ct+k_pm(t))
$$

Si:

$$
m(t)=A_m\cos(\omega_mt)
$$

entonces:

$$
s_{PM}(t)=A_c\cos(\omega_ct+\beta\cos(\omega_mt))
$$

donde:

$$
\beta=k_pA_m
$$

#### 9.2 PM de banda angosta

Condición:

$$
\beta\ll 1
$$

Aproximaciones:

$$
\cos(\beta m(t))\approx 1
$$

$$
\sin(\beta m(t))\approx \beta m(t)
$$

Demostración:

Partimos de:

$$
s_{PM}(t)=A_c\cos(\omega_ct+\beta m(t))
$$

Usamos:

$$
\cos(a+b)=\cos(a)\cos(b)-\sin(a)\sin(b)
$$

Entonces:

$$
s_{PM}(t)
=
A_c\cos(\omega_ct)\cos(\beta m(t))
-
A_c\sin(\omega_ct)\sin(\beta m(t))
$$

Con \(\beta\ll1\):

$$
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
A_c\beta m(t)\sin(\omega_ct)
$$

Para:

$$
m(t)=\cos(\omega_mt)
$$

se obtiene:

$$
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
A_c\beta \cos(\omega_mt)\sin(\omega_ct)
$$

Usando:

$$
\sin(a)\cos(b)=\frac{1}{2}[\sin(a+b)+\sin(a-b)]
$$

llegamos a:

$$
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
\frac{A_c\beta}{2}
[
\sin((\omega_c+\omega_m)t)
+
\sin((\omega_c-\omega_m)t)
]
$$

Aparecen principalmente:

$$
\omega_c-\omega_m
$$

$$
\omega_c
$$

$$
\omega_c+\omega_m
$$

Ancho de banda:

$$
B_{NBPM}\approx 2f_m
$$

#### 9.3 PM de banda ancha

Condición:

$$
\beta>1
$$

Componentes espectrales:

$$
\omega=\omega_c\pm n\omega_m
$$

con:

$$
n=1,2,3,\dots
$$

Ancho de banda teórico:

$$
B_{WBPM,\text{teórico}}\to\infty
$$

Regla de Carson:

$$
B_{Carson}\approx 2(\Delta f+f_m)
$$

Para PM sinusoidal:

$$
\Delta f=\beta f_m
$$

Entonces:

$$
B_{Carson,PM}\approx 2(\beta+1)f_m
$$

### 3. Significado de las variables

- \(s_{PM}(t)\): señal modulada en fase.
- \(A_c\): amplitud de portadora.
- \(\omega_c\): frecuencia angular de portadora.
- \(m(t)\): mensaje.
- \(k_p\): sensibilidad de fase, mide cuántos radianes cambia la fase por unidad de mensaje.
- \(A_m\): amplitud del mensaje sinusoidal.
- \(\omega_m\): frecuencia angular del mensaje.
- \(\beta\): índice de fase.
- \(\Delta f\): desviación máxima de frecuencia.
- \(f_m\): frecuencia máxima del mensaje sinusoidal.
- \(B_{Carson}\): ancho de banda práctico estimado.

### 4. ¿Por qué se hace esto?

En PM, el mensaje entra dentro del argumento del coseno. Eso significa que modifica la fase instantánea:

$$
\phi(t)=\omega_ct+k_pm(t)
$$

No modifica directamente la amplitud \(A_c\). Por eso PM ideal tiene envolvente constante.

Para \(\beta\ll1\), la aproximación de ángulo pequeño permite linealizar la señal. El resultado muestra una portadora fuerte y un término proporcional a \(m(t)\sin(\omega_ct)\). Ese segundo término es una modulación DSB-SC, pero en cuadratura porque usa seno en vez de coseno.

Para \(\beta>1\), la aproximación ya no sirve porque los términos de orden superior ya no son despreciables. Aparecen múltiples bandas laterales alrededor de \(\omega_c\).

### 5. Interpretación física

Modular fase significa adelantar o atrasar los cruces de la portadora según el mensaje. La amplitud ideal no sube ni baja; lo que cambia es la posición temporal de los ciclos.

En PM de banda angosta, la señal se parece a una portadora casi pura con pequeñas bandas laterales. Por eso su ancho de banda aproximado es parecido al de AM/DSB con mensaje sinusoidal: \(2f_m\).

En PM de banda ancha, la fase varía mucho. El espectro contiene componentes en \(\omega_c\pm n\omega_m\) para muchos valores de \(n\). Teóricamente hay infinitas bandas laterales, aunque en la práctica las de potencia muy pequeña se ignoran.

La regla de Carson estima un ancho de banda útil que contiene la mayor parte de la potencia. No dice que no haya energía fuera; dice que fuera de esa banda suele ser despreciable para diseño práctico.

### 6. Recuadro importante

> [!IMPORTANT]
> PM de banda angosta se aproxima como una portadora más un término DSB-SC en cuadratura. PM de banda ancha tiene múltiples bandas laterales y se estima con Carson.
> Memoriza: \(s_{PM}(t)\), \(\beta=k_pA_m\), condición \(\beta\ll1\), condición \(\beta>1\) y Carson.
> Debes saber demostrar: la aproximación NBPM desde \(\cos(a+b)\) y la aparición de bandas laterales.

> [!NOTE]
> En PM ideal, la amplitud \(A_c\) permanece constante; la información está en la fase.

> [!WARNING]
> No uses la aproximación \(\sin(x)\approx x\), \(\cos(x)\approx1\) si \(\beta\) no es pequeño.

> [!TIP]
> Para identificar NBPM en parcial, busca \(\beta\ll1\) y desarrolla con \(\cos(a+b)\).

### 7. Preguntas tipo parcial

1. Demuestre la aproximación de PM de banda angosta.
2. Explique por qué NBPM se relaciona con DSB-SC.
3. Use Carson para estimar el ancho de banda de una señal PM.
4. Explique por qué PM de banda ancha tiene ancho de banda teórico infinito.

### 8. Errores comunes

1. Confundir índice de fase \(\beta\) con índice de modulación AM \(\mu\).
2. Decir que PM ideal cambia amplitud.
3. Aplicar Carson como si fuera ancho de banda exacto.
4. Olvidar que el término DSB-SC de NBPM está en cuadratura.

### 9. Mini-ejemplo

Si \(m(t)=\cos(2\pi 1\,\text{kHz}\,t)\) y \(\beta=0.1\), entonces \(\beta\ll1\) y la señal es NBPM.

El ancho de banda aproximado es:

$$
B_{NBPM}\approx 2f_m=2(1\,\text{kHz})=2\,\text{kHz}
$$

Si \(\beta=5\), se considera PM de banda ancha. Por Carson:

$$
B_{Carson,PM}\approx 2(\beta+1)f_m
=2(6)(1\,\text{kHz})=12\,\text{kHz}
$$

Ejercicios guía:

1. Demuestre la expansión de NBPM paso a paso.
2. Calcule \(B_{Carson}\) para \(\beta=3\) y \(f_m=5\text{ kHz}\).
3. Explique por qué aparecen bandas laterales de orden \(n\).

---

# Cómo se conectan todos los temas

1. Primero se define una señal aleatoria como un proceso con muchas realizaciones posibles.
2. Luego se calculan sus estadísticas: media, varianza, RMS, potencia y momentos.
3. Si el proceso es WSS, la media es constante y la autocorrelación depende solo de \(\tau\).
4. Con la autocorrelación \(R_x(\tau)\) se obtiene la PSD \(S_x(\omega)\) mediante Wiener-Khinchine.
5. Con la PSD se calcula potencia total, potencia en bandas específicas y ancho de banda.
6. Si la señal pasa por un sistema LTI, su PSD cambia por \(|H(\omega)|^2\).
7. Si hay ruido AWGN independiente, la PSD total suma señal más ruido: \(S_y(\omega)=S_x(\omega)+S_n(\omega)\).
8. En modulación DSB, multiplicar por la portadora traslada el espectro del mensaje alrededor de \(\pm\omega_c\).
9. En demodulación coherente, se multiplica otra vez por la portadora y se usa un filtro pasa bajas para recuperar banda base.
10. En PM de banda angosta, la aproximación de ángulo pequeño muestra una portadora más un término DSB-SC en cuadratura.
11. En PM de banda ancha, la aproximación deja de servir y aparecen múltiples bandas laterales; Carson estima el ancho de banda práctico.

> [!IMPORTANT]
> La ruta mental del parcial es: estadística temporal \(\rightarrow\) autocorrelación \(\rightarrow\) PSD \(\rightarrow\) potencia/ancho de banda \(\rightarrow\) sistemas LTI \(\rightarrow\) ruido \(\rightarrow\) modulación/demodulación.

> [!TIP]
> Si un problema mezcla temas, identifica primero qué dominio conviene: tiempo para media/autocorrelación, frecuencia para PSD/filtros/ancho de banda.

---

# Preguntas finales tipo parcial

1. Dada una señal aleatoria, calcule media, varianza, RMS y potencia.
2. Determine si un proceso es WSS.
3. Calcule la autocorrelación de una señal senoidal con fase aleatoria.
4. Obtenga la PSD a partir de una ACF exponencial.
5. Obtenga la ACF a partir de una PSD rectangular.
6. Explique por qué el ruido blanco ideal tiene ancho de banda infinito.
7. Compare ruido blanco ideal y ruido limitado en banda.
8. Calcule la PSD de salida de un filtro LTI.
9. Analice una señal DSB en tiempo, autocorrelación, PSD y ancho de banda.
10. Explique cómo afecta un canal AWGN a la PSD.
11. Compare demodulación convencional y demodulación con Hilbert.
12. Demuestre que PM de banda angosta se relaciona con DSB.
13. Demuestre que PM de banda ancha tiene ancho de banda teóricamente infinito.
14. Use la regla de Carson para estimar ancho de banda de PM.
15. Explique qué pasa con la autocorrelación y la PSD cuando una señal pasa por un filtro pasa bajas.
16. Explique por qué una autocorrelación angosta implica una PSD ancha.
17. Compare estimador sesgado y no sesgado de autocorrelación.
18. Explique por qué Welch reduce la varianza de la PSD estimada.
19. Explique la diferencia física entre potencia, varianza y RMS.
20. Analice el efecto de sobremodulación en AM convencional.

---

# Qué memorizar y qué saber demostrar

## Debo memorizar

- \(P_x=R_x(0)=E\{|x(t)|^2\}\).
- \(K_x(0)=\sigma_x^2\).
- Para WSS: \(E\{x(t)\}=\mu_x\) constante y \(R_x(t_1,t_2)=R_x(\tau)\).
- Wiener-Khinchine: \(S_x(\omega)=\mathcal{F}\{R_x(\tau)\}\).
- Sistema LTI: \(S_y(\omega)=|H(\omega)|^2S_x(\omega)\).
- Ruido blanco: \(S_n(\omega)=N_0/2\), \(R_n(\tau)=N_0\delta(\tau)/2\).
- DSB-SC: \(s(t)=A_cm(t)\cos(\omega_ct)\), \(B_{DSB}=2B_m\).
- AM convencional: \(s(t)=A_c[1+\mu m_n(t)]\cos(\omega_ct)\), sin sobremodulación si \(0\leq\mu\leq1\).
- NBPM: \(s_{PM}(t)\approx A_c\cos(\omega_ct)-A_c\beta m(t)\sin(\omega_ct)\).
- Carson para PM: \(B_{Carson,PM}\approx2(\beta+1)f_m\).

## Debo saber demostrar

- \(P_x=\sigma_x^2+\mu_x^2\) para señales reales.
- \(R_x(0)=P_x\).
- \(K_x(0)=\sigma_x^2\).
- La ACF del seno con fase aleatoria.
- La PSD de la ACF exponencial.
- La ACF de una PSD rectangular.
- Que multiplicar por una portadora traslada el espectro.
- La demodulación coherente de DSB-SC usando \(\cos^2(\omega_ct)\).
- La aproximación de PM de banda angosta usando \(\cos(a+b)\).
- Por qué la PSD de salida de un LTI usa \(|H(\omega)|^2\).

> [!WARNING]
> En el parcial no basta con escribir fórmulas. Acompaña cada resultado con una interpretación: potencia, memoria temporal, ancho de banda, ruido o recuperación del mensaje.
