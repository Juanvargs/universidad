# Repaso parcial 1

Este apunte está escrito como un texto de teoría para estudiar el parcial. La intención no es memorizar fórmulas sueltas, sino entender de dónde salen, por qué se usan y qué significan físicamente.

> [!IMPORTANT]
> En Teoría de Señales casi todo se conecta por una ruta: estadística de la señal, autocorrelación, PSD, paso por sistemas LTI, ruido, modulación y demodulación.

---

# 1. Señales aleatorias y estadística básica

## 1.1 Por qué aparece la idea de señal aleatoria

En señales deterministas, una función como $x(t)=\cos(2\pi f_0t)$ queda completamente definida. Si conozco la fórmula, conozco el valor de la señal en cualquier instante.

Pero muchas señales reales no se pueden describir así. El ruido térmico, interferencias, tráfico de comunicaciones, fluctuaciones de sensores o señales medidas en laboratorio cambian de una observación a otra. No significa que no podamos estudiarlas; significa que debemos describirlas mediante estadísticas.

Un **proceso aleatorio** es un modelo que produce señales posibles. Cada vez que ocurre el experimento aleatorio aparece una señal temporal distinta. A cada una de esas señales se le llama **realización**.

Por ejemplo, si mido ruido en un resistor durante diez segundos, obtengo una curva. Si repito la medición, obtengo otra curva. Ninguna curva aislada es "todo el proceso"; cada una es una realización.

## 1.2 Proceso, realización, observación y ensamble

Un proceso aleatorio se puede imaginar como una colección de señales:

```math
x(t,\zeta)
```

donde $t$ es el tiempo y $\zeta$ representa el resultado aleatorio del experimento. Si fijo $\zeta=\zeta_1$, obtengo una realización:

```math
x(t,\zeta_1)
```

Si fijo el tiempo $t=t_0$, entonces $x(t_0,\zeta)$ es una variable aleatoria, porque cambia entre realizaciones.

El **ensamble** es el conjunto de todas las realizaciones posibles. El **promedio de ensamble** promedia entre realizaciones para un mismo instante. El **promedio temporal** promedia una sola realización a lo largo del tiempo.

Una **señal aleatoria continua** se define para todo tiempo continuo:

```math
x(t)
```

Una **señal aleatoria discreta** se define solo en instantes enteros:

```math
x[n]
```

## 1.3 Esperanza matemática y momentos

La herramienta central para describir procesos aleatorios es la esperanza matemática. La esperanza no es un valor instantáneo de la señal; es un promedio estadístico.

La media de un proceso es:

```math
\mu_x(t)=E\{x(t)\}
```

Esta fórmula dice: tome el valor de la señal en el instante $t$, mire todos los valores posibles entre realizaciones y calcule su promedio.

Los momentos generalizan esta idea:

```math
m_n(t)=E\{x^n(t)\}
```

El primer momento es la media. El segundo momento, $E\{x^2(t)\}$, es muy importante porque se relaciona con potencia, RMS y varianza.

## 1.4 Media, componente DC y significado físico

La media:

```math
\mu_x(t)=E\{x(t)\}
```

representa el nivel promedio de la señal. En señales eléctricas, se interpreta como componente DC. Si la media es cero, la señal oscila alrededor de cero. Si la media es positiva o negativa, hay un desplazamiento constante.

La componente DC es:

```math
x_{DC}=\mu_x
```

La componente alternante se obtiene quitando esa parte constante:

```math
x_{AC}(t)=x(t)-\mu_x
```

La razón de separar DC y AC es que una señal puede tener potencia por estar desplazada y también por estar fluctuando. La media mide el desplazamiento; la varianza mide la fluctuación.

## 1.5 Varianza y desviación estándar

La varianza mide qué tan lejos se alejan los valores de la señal respecto a su media:

```math
\sigma_x^2(t)=E\{(x(t)-\mu_x(t))^2\}
```

Primero se resta la media para medir solo la fluctuación. Luego se eleva al cuadrado para evitar cancelación entre valores positivos y negativos. Finalmente se promedia estadísticamente.

La desviación estándar es:

```math
\sigma_x(t)=\sqrt{\sigma_x^2(t)}
```

Tiene las mismas unidades que la señal. Por eso suele ser más fácil de interpretar que la varianza.

Físicamente, una varianza grande significa que la señal cambia mucho alrededor de su nivel promedio. Una varianza pequeña significa que la señal está más concentrada cerca de la media.

## 1.6 Potencia promedio y RMS

En señales, la potencia se asocia con el promedio cuadrático. Para procesos aleatorios:

```math
P_x=E\{|x(t)|^2\}
```

Si la señal es real, esto se reduce a:

```math
P_x=E\{x^2(t)\}
```

El valor RMS es:

```math
x_{RMS}=\sqrt{E\{x^2(t)\}}
```

Por tanto, para señales reales:

```math
x_{RMS}=\sqrt{P_x}
```

El RMS se llama valor eficaz porque una señal alterna con cierto RMS produce la misma potencia promedio que una señal constante de ese valor.

## 1.7 De dónde sale la relación entre potencia, media y varianza

La relación que queremos entender es:

```math
P_x=\sigma_x^2+\mu_x^2
```

Para ver la conexión entre potencia, media y varianza, escribimos:

```math
x(t)=\mu_x+\bigl(x(t)-\mu_x\bigr)
```

Ahora elevamos al cuadrado:

```math
x^2(t)=\mu_x^2+2\mu_x\bigl(x(t)-\mu_x\bigr)+\bigl(x(t)-\mu_x\bigr)^2
```

Aplicando esperanza:

```math
E\{x^2(t)\}
=
\mu_x^2
+
2\mu_x E\{x(t)-\mu_x\}
+
E\{(x(t)-\mu_x)^2\}
```

Pero:

```math
E\{x(t)-\mu_x\}=E\{x(t)\}-\mu_x=0
```

Entonces:

```math
P_x=E\{x^2(t)\}=\mu_x^2+\sigma_x^2
```

Esta ecuación explica físicamente que la potencia total tiene dos partes: potencia DC y potencia alternante.

## 1.8 Proceso de segundo orden

Un proceso es de **segundo orden** si existen y son finitos los momentos necesarios hasta orden dos:

```math
E\{x(t)\}
```

```math
E\{|x(t)|^2\}
```

Esto importa porque la mayor parte del curso trabaja con media, varianza, autocorrelación y PSD. Todas esas cantidades necesitan que el segundo momento exista.

> [!IMPORTANT]
> Media, varianza, RMS y potencia no son lo mismo. La media mide nivel DC; la varianza mide fluctuación; la potencia mide promedio cuadrático total; el RMS es la raíz de la potencia.

> [!WARNING]
> Si $\mu_x\neq0$, no puedes decir que potencia y varianza son iguales.

> [!TIP]
> En un problema, primero calcula $E\{x(t)\}$. Después decide si necesitas dispersión $\sigma_x^2$, potencia $P_x$ o valor eficaz $x_{RMS}$.

## Preguntas tipo parcial

1. Explique la diferencia entre proceso aleatorio, realización, observación y ensamble.
2. Dada una señal aleatoria, calcule media, varianza, RMS y potencia.
3. Demuestre que $P_x=\sigma_x^2+\mu_x^2$.
4. Interprete físicamente una señal con media cero y varianza alta.

## Mini-ejemplo

Sea:

```math
x(t)=3+n(t)
```

donde $n(t)$ tiene media cero y varianza $4$. Entonces:

```math
\mu_x=3
```

```math
\sigma_x^2=4
```

```math
P_x=\mu_x^2+\sigma_x^2=9+4=13
```

```math
x_{RMS}=\sqrt{13}
```

La señal tiene una componente DC igual a $3$ y una componente aleatoria cuya potencia alternante es $4$.

---

# 2. Estacionariedad y ergodicidad

## 2.1 El problema de las estadísticas que cambian con el tiempo

Un proceso aleatorio puede tener estadísticas que cambian con el tiempo. Por ejemplo, el ruido de un circuito durante el encendido no tiene necesariamente las mismas propiedades que después de alcanzar régimen permanente.

La **estacionariedad** describe procesos cuyas estadísticas no cambian al mover el origen temporal. En su forma completa, exige que todas las distribuciones estadísticas sean invariantes ante desplazamientos temporales. Esa definición es fuerte y difícil de verificar.

Por eso en señales se usa mucho la **estacionariedad en sentido amplio**, llamada WSS. Es una versión basada solo en estadísticas de primer y segundo orden.

## 2.2 Proceso WSS

Un proceso es WSS si cumple dos condiciones.

Primera condición: media constante.

```math
E\{x(t)\}=\mu_x=\text{constante}
```

Esto significa que el nivel promedio no depende del instante.

Segunda condición: la autocorrelación depende solo del retardo.

```math
R_x(t_1,t_2)=R_x(t_2-t_1)=R_x(\tau)
```

La autocorrelación general compara la señal en dos tiempos $t_1$ y $t_2$. En un proceso WSS no importa dónde están esos tiempos en el eje temporal; solo importa qué tan separados están.

El retardo es:

```math
\tau=t_2-t_1
```

## 2.3 Por qué WSS simplifica todo

Si un proceso no es WSS, la autocorrelación depende de dos variables:

```math
R_x(t_1,t_2)
```

Eso hace que el análisis sea más difícil, porque la relación estadística entre valores de la señal cambia con el tiempo absoluto.

Si el proceso es WSS, basta con una variable:

```math
R_x(\tau)
```

Esto permite hablar de memoria temporal, potencia y PSD de manera estable. De hecho, el teorema de Wiener-Khinchine se formula para procesos WSS.

## 2.4 Promedio de ensamble y promedio temporal

El promedio de ensamble se calcula mirando muchas realizaciones en el mismo instante. La media estadística es:

```math
\mu_x=E\{x(t)\}
```

Pero en laboratorio normalmente no tenemos infinitas realizaciones. Tenemos una medición larga. Entonces usamos el promedio temporal:

```math
\mu_x=
\lim_{T\to\infty}
\frac{1}{T}
\int_0^T x(t)\,dt
```

Para autocorrelación temporal:

```math
R_x(\tau)=
\lim_{T\to\infty}
\frac{1}{T}
\int_0^T x(t)x^*(t+\tau)\,dt
```

Estas fórmulas dicen que una sola realización puede revelar estadísticas del proceso si el proceso es ergódico.

## 2.5 Ergodicidad

Un proceso es **ergódico** si sus estadísticas de ensamble pueden obtenerse a partir de una sola realización suficientemente larga.

La ergodicidad no es solo una comodidad matemática. Es la razón por la que podemos medir una señal durante un tiempo largo y estimar su media, autocorrelación o PSD como si representara al proceso completo.

## 2.6 Relación entre estacionariedad y ergodicidad

Todo proceso ergódico debe ser estacionario en la estadística que se está estimando. Si una sola realización larga representa al ensamble, entonces la estadística no puede depender del punto de inicio temporal.

Pero no todo proceso estacionario es ergódico.

Un ejemplo clásico es:

```math
x(t)=A
```

donde $A$ es una variable aleatoria constante en el tiempo. Cada realización es una línea horizontal. El proceso es estacionario porque no cambia con el tiempo, pero una sola realización solo muestra un valor de $A$. No permite conocer toda la distribución de $A$. Por eso no es ergódico.

## 2.7 Cuándo puedo usar una sola realización

Puedes usar una sola realización para estimar estadísticas cuando el proceso puede asumirse ergódico. En la práctica, esto se asume si:

- El proceso está en régimen permanente.
- Las estadísticas no parecen cambiar con el tiempo.
- La observación es suficientemente larga.
- El sistema físico que genera la señal no cambia durante la medición.

> [!IMPORTANT]
> WSS significa estabilidad de media y autocorrelación. Ergodicidad significa que una realización larga puede reemplazar al ensamble.

> [!WARNING]
> WSS no garantiza ergodicidad. Estacionario no significa automáticamente "estimable con una sola medición".

> [!NOTE]
> En datos medidos, la ergodicidad es la hipótesis que permite pasar de teoría estadística a estimación práctica.

## Preguntas tipo parcial

1. Determine si un proceso es WSS calculando media y autocorrelación.
2. Explique por qué WSS no implica ergodicidad.
3. Explique por qué un proceso ergódico debe ser estacionario.
4. Diga cuándo es válido estimar $R_x(\tau)$ con una sola realización.

## Mini-ejemplo

Sea:

```math
x(t)=A\cos(\omega_0t+\phi)
```

con $\phi$ uniforme en $[0,2\pi)$. La media es:

```math
E\{x(t)\}=0
```

La autocorrelación resulta:

```math
R_x(t_1,t_2)=\frac{A^2}{2}\cos(\omega_0(t_2-t_1))
```

Como depende solo de $t_2-t_1$, el proceso es WSS.

---

# 3. Autocorrelación, covarianza y correlación cruzada

## 3.1 La idea de memoria temporal

Una señal aleatoria no solo se caracteriza por su amplitud promedio o su potencia. También importa si sus valores en instantes cercanos se parecen.

La **autocorrelación** mide esa semejanza. Si $x(t)$ y $x(t+\tau)$ son parecidos, la autocorrelación para ese retardo será grande. Si no guardan relación, será pequeña.

Por eso se dice que la autocorrelación mide la **memoria** de la señal.

## 3.2 Definición general de autocorrelación

Para un proceso complejo:

```math
R_x(t_1,t_2)=E\{x(t_1)x^*(t_2)\}
```

Se multiplica el valor de la señal en $t_1$ por el conjugado del valor en $t_2$, y luego se promedia. El conjugado aparece para que la potencia de señales complejas quede positiva:

```math
x(t)x^*(t)=|x(t)|^2
```

Para procesos WSS:

```math
R_x(\tau)=E\{x(t)x^*(t+\tau)\}
```

## 3.3 Por qué la autocorrelación en cero es potencia

La afirmación clave es:

```math
R_x(0)=P_x
```

Si el retardo es cero:

```math
R_x(0)=E\{x(t)x^*(t)\}
```

Como:

```math
x(t)x^*(t)=|x(t)|^2
```

entonces:

```math
R_x(0)=E\{|x(t)|^2\}=P_x
```

Esta es una de las conexiones más importantes del curso: la autocorrelación en el origen es la potencia promedio.

## 3.4 Covarianza

La autocorrelación incluye la componente DC. Si queremos estudiar solo las fluctuaciones, restamos la media:

```math
K_x(\tau)=E\{(x(t)-\mu_x)(x^*(t+\tau)-\mu_x^*)\}
```

Al desarrollar:

```math
K_x(\tau)=R_x(\tau)-|\mu_x|^2
```

En $\tau=0$:

```math
K_x(0)=\sigma_x^2
```

Entonces, autocorrelación en cero da potencia total; covarianza en cero da varianza.

## 3.5 Correlación cruzada

La correlación cruzada compara dos procesos:

```math
R_{xy}(\tau)=E\{x(t)y^*(t+\tau)\}
```

Sirve para detectar retardos, sincronización, similitud entre entrada y salida, o presencia de una señal conocida dentro de otra.

Si $R_{xy}(\tau)$ tiene un máximo en cierto retardo, eso sugiere que una señal se parece a la otra desplazada ese tiempo.

## 3.6 Propiedades de la autocorrelación

La autocorrelación cumple:

```math
|R_x(\tau)|\leq R_x(0)
```

La mayor semejanza ocurre cuando la señal se compara consigo misma sin retardo.

También cumple simetría conjugada:

```math
R_x(\tau)=R_x^*(-\tau)
```

Para señales reales:

```math
R_x(\tau)=R_x(-\tau)
```

Por eso la ACF de una señal real WSS debe ser una función par.

## 3.7 Diferencia cuadrática media

Otra forma de medir cuánto cambia la señal al pasar un retardo es:

```math
\varepsilon_x(\tau)=E\{(x(t)-x(t+\tau))^2\}
```

Para señales reales WSS:

```math
\varepsilon_x(\tau)=2R_x(0)-2R_x(\tau)
```

Esta expresión sale al expandir el cuadrado:

```math
(x(t)-x(t+\tau))^2=x^2(t)+x^2(t+\tau)-2x(t)x(t+\tau)
```

Como el proceso es WSS, $E\{x^2(t)\}=E\{x^2(t+\tau)\}=R_x(0)$, y el término cruzado es $R_x(\tau)$.

## 3.8 Intervalo y tiempo efectivo de correlación

El intervalo de correlación define a partir de qué retardo la señal se considera casi no correlacionada:

```math
\frac{R_x(\tau_c)}{\sigma_x^2}=\beta
```

con:

```math
\beta \approx 0.05 \text{ a } 0.1
```

El tiempo efectivo de correlación resume el ancho de la autocorrelación:

```math
\Delta \tau_e=
\frac{1}{R_x(0)}
\int_0^\infty R_x(\tau)\,d\tau
```

Si este valor es grande, la señal tiene memoria larga. Si es pequeño, la señal cambia rápidamente.

## 3.9 Ejemplos típicos

Ruido blanco:

```math
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
```

Esto significa que solo hay correlación en el mismo instante. Para retardos distintos de cero, el ruido no tiene memoria.

Seno con fase aleatoria:

```math
x(t)=a\cos(\omega_0t+\phi)
```

con $\phi$ uniforme. Su autocorrelación es:

```math
R_x(\tau)=\frac{a^2}{2}\cos(\omega_0\tau)
```

La ACF es periódica porque la señal contiene una oscilación periódica.

ACF exponencial:

```math
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
```

Si $\alpha$ aumenta, la exponencial decae más rápido. Eso significa menor memoria temporal y mayor dispersión espectral.

> [!IMPORTANT]
> La autocorrelación mide memoria. $R_x(0)$ es potencia. $K_x(0)$ es varianza.

> [!WARNING]
> Si la señal es compleja, no olvides el conjugado.

> [!TIP]
> Si una ACF real no es par, probablemente hay un error de cálculo.

## Preguntas tipo parcial

1. Explique por qué $R_x(0)=P_x$.
2. Diferencie autocorrelación y covarianza.
3. Calcule la diferencia cuadrática media usando $R_x(\tau)$.
4. Interprete una ACF periódica, una ACF angosta y una ACF exponencial.

## Mini-ejemplo

Para:

```math
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
```

la potencia es:

```math
P_x=R_x(0)=\sigma_x^2
```

y el tiempo efectivo es:

```math
\Delta\tau_e=
\frac{1}{\sigma_x^2}
\int_0^\infty \sigma_x^2e^{-\alpha\tau}\,d\tau
=
\frac{1}{\alpha}
```

Por eso $\alpha$ controla directamente qué tan rápido se pierde la memoria.

---

# 4. PSD, espectro estadístico y teorema de Wiener-Khinchine

## 4.1 Por qué necesitamos un espectro estadístico

Para señales deterministas se usa la transformada de Fourier para saber qué frecuencias contiene la señal. En procesos aleatorios, una realización aislada puede cambiar, así que no se describe la señal exacta sino cómo se distribuye su potencia en frecuencia.

La **densidad espectral de potencia** o **PSD** indica cuánta potencia hay por unidad de frecuencia angular.

Si $S_x(\omega)$ es grande cerca de una frecuencia, el proceso tiene mucha potencia alrededor de esa frecuencia.

## 4.2 Definición de PSD

La definición formal usa una observación truncada $X_T(\omega)$:

```math
S_x(\omega)=
\lim_{T\to\infty}
\frac{E\{|X_T(\omega)|^2\}}{2T}
```

La magnitud cuadrada aparece porque estamos midiendo potencia. El promedio estadístico aparece porque el proceso es aleatorio. La división por $2T$ convierte energía observada en potencia promedio.

## 4.3 Potencia total como área bajo la PSD

Como la PSD es potencia por frecuencia, la potencia total se obtiene integrando:

```math
P_x=R_x(0)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
```

El factor $1/(2\pi)$ aparece porque se está usando frecuencia angular $\omega$, no frecuencia en Hz.

Si la media es cero:

```math
\sigma_x^2=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
```

Esto ocurre porque con media cero la potencia coincide con la varianza.

## 4.4 Propiedades de la PSD

La PSD es real:

```math
S_x(\omega)\in \mathbb{R}
```

No puede ser negativa:

```math
S_x(\omega)\geq 0
```

La razón física es que representa densidad de potencia. La razón matemática es que viene de un promedio de magnitudes cuadradas:

```math
E\{|X_T(\omega)|^2\}\geq0
```

Para procesos reales:

```math
S_x(\omega)=S_x(-\omega)
```

El espectro es par porque las frecuencias positivas y negativas son conjugadas en señales reales.

## 4.5 Teorema de Wiener-Khinchine

Para procesos WSS, la autocorrelación y la PSD forman un par de Fourier:

```math
S_x(\omega)=\mathcal{F}\{R_x(\tau)\}
```

```math
R_x(\tau)=\mathcal{F}^{-1}\{S_x(\omega)\}
```

En forma integral:

```math
S_x(\omega)=
\int_{-\infty}^{\infty}
R_x(\tau)e^{-j\omega\tau}\,d\tau
```

```math
R_x(\tau)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}
S_x(\omega)e^{j\omega\tau}\,d\omega
```

Este teorema es profundo: dice que la forma en que una señal se parece a sí misma en el tiempo determina cómo está repartida su potencia en frecuencia.

## 4.6 Forma con coseno

Si $R_x(\tau)$ es real y par:

```math
S_x(\omega)=
2\int_0^\infty R_x(\tau)\cos(\omega\tau)\,d\tau
```

Y si $S_x(\omega)$ es real y par:

```math
R_x(\tau)=
\frac{1}{\pi}
\int_0^\infty S_x(\omega)\cos(\omega\tau)\,d\omega
```

Se usa coseno porque la parte seno se cancela por simetría impar.

## 4.7 Pares ACF-PSD frecuentes

ACF exponencial:

```math
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
```

PSD:

```math
S_x(\omega)=
\frac{2\sigma_x^2\alpha}{\alpha^2+\omega^2}
```

Cuando $\alpha$ aumenta, la ACF se angosta y la PSD se ensancha.

Ruido blanco:

```math
S_n(\omega)=\frac{N_0}{2}
```

```math
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
```

PSD rectangular:

```math
S_n(\omega)=
\begin{cases}
\dfrac{N_0}{2}, & |\omega|<B\\
0, & \text{otro caso}
\end{cases}
```

ACF correspondiente:

```math
R_n(\tau)=
\frac{N_0B}{2\pi}
\operatorname{sinc}
\left(
\frac{B\tau}{\pi}
\right)
```

Aquí se usa:

```math
\operatorname{sinc}(u)=\frac{\sin(\pi u)}{\pi u}
```

Seno con fase aleatoria:

```math
R_x(\tau)=
\frac{a^2}{2}\cos(\omega_0\tau)
```

PSD:

```math
S_x(\omega)=
\frac{\pi a^2}{2}
\left[
\delta(\omega-\omega_0)+\delta(\omega+\omega_0)
\right]
```

Las deltas indican potencia concentrada exactamente en $\pm\omega_0$.

## 4.8 Ancho temporal contra ancho espectral

Una autocorrelación angosta implica que la señal pierde memoria rápidamente. Eso requiere muchas frecuencias para describir cambios rápidos. Por eso el espectro es ancho.

Una autocorrelación ancha implica que la señal cambia lentamente y conserva memoria. Entonces la potencia se concentra en un rango más estrecho de frecuencias.

Esta idea es una manifestación estadística de la relación tiempo-frecuencia de Fourier.

> [!IMPORTANT]
> Para procesos WSS, $S_x(\omega)$ es la transformada de Fourier de $R_x(\tau)$. La potencia total es el área bajo la PSD dividida por $2\pi$.

> [!WARNING]
> La PSD no es amplitud espectral. Es potencia por unidad de frecuencia.

> [!TIP]
> Si ves una ACF muy estrecha, espera una PSD ancha. Si ves una ACF ancha, espera una PSD estrecha.

## Preguntas tipo parcial

1. Obtenga la PSD de una ACF exponencial.
2. Obtenga la ACF de una PSD rectangular.
3. Explique por qué la PSD no puede ser negativa.
4. Explique la relación entre ancho de autocorrelación y ancho de espectro.

## Mini-ejemplo

Dada:

```math
R_x(\tau)=\sigma_x^2e^{-\alpha|\tau|}
```

Como es par:

```math
S_x(\omega)=
2\int_0^\infty \sigma_x^2e^{-\alpha\tau}\cos(\omega\tau)\,d\tau
```

Usando:

```math
\int_0^\infty e^{-\alpha\tau}\cos(\omega\tau)\,d\tau
=
\frac{\alpha}{\alpha^2+\omega^2}
```

queda:

```math
S_x(\omega)=
\frac{2\sigma_x^2\alpha}{\alpha^2+\omega^2}
```

---

# 5. Estimadores de ACF y PSD

## 5.1 Por qué estimar no es lo mismo que calcular la teoría

Las fórmulas teóricas usan esperanzas, límites infinitos o autocorrelaciones exactas. En un experimento real tenemos una secuencia finita:

```math
x[0],x[1],\dots,x[N-1]
```

Por eso usamos estimadores. Un estimador es una fórmula que intenta aproximar una cantidad teórica usando datos disponibles.

## 5.2 Autocorrelación muestral

Para retardo discreto $k$, comparamos $x[n]$ con $x[n+k]$. Como la secuencia tiene longitud finita, para retardos grandes hay menos pares disponibles.

Estimador sesgado:

```math
\hat{R}_{x,\text{sesgada}}[k]
=
\frac{1}{N}
\sum_{n=0}^{N-1-k}
x[n]x^*[n+k]
```

Estimador no sesgado:

```math
\hat{R}_{x,\text{no sesgada}}[k]
=
\frac{1}{N-k}
\sum_{n=0}^{N-1-k}
x[n]x^*[n+k]
```

El estimador sesgado divide siempre por $N$. El no sesgado divide por el número real de productos disponibles, $N-k$.

## 5.3 Sesgo contra varianza

El estimador no sesgado corrige el promedio esperado, pero en retardos grandes usa pocos datos y puede fluctuar mucho.

El estimador sesgado tiende a subestimar la autocorrelación en retardos grandes, pero suele ser más estable.

En análisis práctico no siempre se escoge el no sesgado. Depende de si importa más corregir el promedio o reducir variabilidad.

## 5.4 Periodograma

La transformada discreta de una secuencia finita es:

```math
X_N(e^{j\omega})=
\sum_{n=0}^{N-1}
x[n]e^{-j\omega n}
```

El periodograma estima la PSD como magnitud cuadrada:

```math
\hat{S}_x(e^{j\omega})
=
\frac{1}{N}
|X_N(e^{j\omega})|^2
```

La FFT no cambia la teoría; solo calcula eficientemente muestras de esta transformada.

## 5.5 Por qué el periodograma tiene alta varianza

El periodograma se calcula a partir de una única observación finita. Para procesos aleatorios, la energía espectral estimada fluctúa mucho de una realización a otra. Aumentar $N$ mejora la resolución en frecuencia, pero el periodograma puede seguir viéndose irregular.

La resolución aproximada es:

```math
\Delta f\approx\frac{f_s}{N}
```

Más muestras permiten distinguir frecuencias más cercanas. Pero resolución y varianza no son lo mismo.

## 5.6 Ventanas y longitud finita

Cuando se toma una señal finita, en realidad se multiplica por una ventana temporal. Multiplicar en tiempo equivale a convolucionar en frecuencia. Por eso aparecen fugas espectrales.

Una ventana rectangular tiene buena resolución pero más fuga. Ventanas como Hann o Hamming reducen fuga, pero ensanchan el lóbulo principal. Eso puede hacer que picos cercanos se mezclen.

## 5.7 Método de Welch

Welch reduce la varianza así:

1. Divide la señal en segmentos.
2. Aplica una ventana a cada segmento.
3. Calcula el periodograma de cada segmento.
4. Promedia los periodogramas.

Al promediar, las fluctuaciones aleatorias se suavizan. Se gana estabilidad, pero se pierde resolución porque cada segmento es más corto que la señal completa.

## 5.8 Comparación entre teoría y experimento

La PSD teórica puede ser lisa, exacta y simétrica. La PSD estimada puede mostrar irregularidades, picos ensanchados, piso de ruido, errores por ventana y fluctuaciones de varianza.

Un pico espectral suele indicar una componente sinusoidal o una banda de potencia concentrada. Un piso espectral indica ruido distribuido. Si se agrega ruido blanco, se eleva el piso de la PSD.

> [!IMPORTANT]
> Estimar una PSD no es obtener la PSD teórica exacta. Los datos finitos introducen varianza, fuga espectral y resolución limitada.

> [!WARNING]
> El periodograma no se vuelve perfectamente suave solo por aumentar $N$.

> [!TIP]
> Si quieres menos varianza en la PSD estimada, usa Welch. Si quieres más resolución, necesitas segmentos más largos o más muestras.

## Preguntas tipo parcial

1. Compare estimador sesgado y no sesgado de ACF.
2. Explique por qué el periodograma tiene alta varianza.
3. Explique cómo Welch reduce varianza.
4. Interprete picos espectrales y piso de ruido.

## Mini-ejemplo

Si $N=1000$ y $k=200$, solo hay $800$ productos $x[n]x^*[n+200]$. El estimador no sesgado divide por $800$, mientras que el sesgado divide por $1000$. El no sesgado corrige el promedio, pero como usa menos información puede fluctuar más.

---

# 6. Ruido blanco, ruido gaussiano y ruido limitado en banda

## 6.1 Blanco y gaussiano no significan lo mismo

"Blanco" describe el espectro. Un ruido es blanco si su PSD es constante. Eso significa que tiene la misma densidad de potencia en todas las frecuencias.

"Gaussiano" describe la distribución de amplitudes. Un ruido es gaussiano si sus valores instantáneos siguen una distribución normal.

Un ruido puede ser gaussiano sin ser blanco, y puede ser blanco sin tener distribución gaussiana. Cuando cumple ambas cosas y se suma a la señal, se llama AWGN.

## 6.2 Ruido blanco ideal

El modelo ideal tiene:

```math
S_n(\omega)=\frac{N_0}{2}, \quad -\infty<\omega<\infty
```

Su autocorrelación es:

```math
R_n(\tau)=\frac{N_0}{2}\delta(\tau)
```

La delta significa que el ruido solo está correlacionado consigo mismo en el mismo instante. Para cualquier retardo distinto de cero, no hay memoria.

## 6.3 Por qué el ruido blanco ideal no existe físicamente

La potencia se obtiene integrando la PSD:

```math
P_n=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}
\frac{N_0}{2}\,d\omega
```

Esa integral diverge porque el ancho de banda es infinito. Por tanto, el ruido blanco ideal tendría potencia infinita. Es útil como modelo local en la banda de interés, no como señal física completa.

## 6.4 Ruido limitado en banda

En un sistema real, el ancho de banda es finito:

```math
S_n(\omega)=
\begin{cases}
\dfrac{N_0}{2}, & |\omega|<\Delta\omega\\
0, & |\omega|\geq \Delta\omega
\end{cases}
```

Su autocorrelación es:

```math
R_n(\tau)=
\frac{N_0\Delta\omega}{2\pi}
\operatorname{sinc}
\left(
\frac{\Delta\omega\tau}{\pi}
\right)
```

Limitar el espectro convierte la delta en una sinc. Eso significa que el ruido filtrado ya tiene cierta correlación temporal.

## 6.5 Potencia de ruido

La potencia es:

```math
P_n=R_n(0)=
\frac{1}{2\pi}
\int_{-\Delta\omega}^{\Delta\omega}
\frac{N_0}{2}\,d\omega
```

Como el intervalo tiene longitud $2\Delta\omega$:

```math
P_n=
\frac{1}{2\pi}
\left(
\frac{N_0}{2}
\right)
(2\Delta\omega)
```

Entonces:

```math
P_n=
\frac{N_0\Delta\omega}{2\pi}
```

Esta fórmula muestra que la potencia de ruido crece linealmente con el ancho de banda.

## 6.6 SNR

La relación señal a ruido es:

```math
SNR=\frac{P_s}{P_n}
```

En decibelios:

```math
SNR_{dB}=
10\log_{10}
\left(
\frac{P_s}{P_n}
\right)
```

Se usa $10\log_{10}$ porque se comparan potencias. Un SNR alto indica señal dominante; un SNR bajo indica que el ruido puede ocultar la señal.

## 6.7 Efectos del ruido

En tiempo, el ruido agrega fluctuaciones. En la PSD, agrega un piso espectral. En autocorrelación, el ruido blanco agrega una delta en el origen; el ruido limitado en banda agrega una sinc.

> [!IMPORTANT]
> Blanco se refiere a PSD. Gaussiano se refiere a amplitudes. AWGN significa aditivo, blanco y gaussiano.

> [!WARNING]
> $N_0/2$ no es potencia total; es densidad espectral de potencia.

> [!TIP]
> Para potencia de ruido, integra la PSD solo sobre la banda que pasa el sistema.

## Preguntas tipo parcial

1. Diferencie ruido blanco y ruido gaussiano.
2. Explique por qué el ruido blanco ideal tiene potencia infinita.
3. Calcule la potencia de ruido limitado en banda.
4. Explique cómo el ruido afecta tiempo, PSD y autocorrelación.

## Mini-ejemplo

Si $N_0=4$ y $\Delta\omega=50\pi$:

```math
P_n=
\frac{N_0\Delta\omega}{2\pi}
=
\frac{4(50\pi)}{2\pi}
=
100
```

Si $P_s=1000$:

```math
SNR=10
```

```math
SNR_{dB}=10\log_{10}(10)=10\text{ dB}
```

---

# 7. Transformación de señales aleatorias por sistemas LTI

## 7.1 Qué hace un sistema LTI

Un sistema LTI es lineal e invariante en el tiempo. Lineal significa que respeta suma y escalamiento. Invariante en el tiempo significa que si se retrasa la entrada, se retrasa la salida en la misma cantidad.

Todo sistema LTI se describe por su respuesta al impulso $h(t)$. La salida se obtiene por convolución:

```math
y(t)=h(t)*x(t)
```

En frecuencia:

```math
Y(\omega)=H(\omega)X(\omega)
```

## 7.2 Qué pasa con la PSD

La PSD de salida es:

```math
S_y(\omega)=|H(\omega)|^2S_x(\omega)
```

Esta fórmula sale de que la salida en frecuencia es $Y=HX$. Si se calcula potencia espectral:

```math
|Y(\omega)|^2=|H(\omega)X(\omega)|^2
```

Entonces:

```math
|Y(\omega)|^2=|H(\omega)|^2|X(\omega)|^2
```

Al promediar estadísticamente, aparece:

```math
S_y(\omega)=|H(\omega)|^2S_x(\omega)
```

Se usa $|H(\omega)|^2$, no $H(\omega)$, porque la PSD mide potencia y la potencia depende de magnitud cuadrada. La fase del filtro no aparece directamente en la PSD.

## 7.3 Potencia de salida

Una vez conocida la PSD de salida:

```math
P_y=R_y(0)=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}
S_y(\omega)\,d\omega
```

El procedimiento siempre es:

1. Hallar $S_x(\omega)$.
2. Multiplicar por $|H(\omega)|^2$.
3. Integrar para obtener potencia.

## 7.4 Filtrado de ruido

Si entra ruido blanco:

```math
S_n(\omega)=\frac{N_0}{2}
```

La salida tiene:

```math
S_y(\omega)=|H(\omega)|^2\frac{N_0}{2}
```

Si $H(\omega)$ es pasa bajas, solo pasa una parte del ruido. Como la potencia es área bajo la PSD, reducir ancho de banda reduce potencia de ruido.

## 7.5 Relación entre filtrado y correlación temporal

Un ruido blanco ideal tiene ACF delta, es decir, no tiene memoria. Al pasar por un filtro pasa bajas, su espectro se vuelve limitado. Una PSD más angosta implica una autocorrelación más ancha. Por eso el ruido filtrado se vuelve más suave y temporalmente correlacionado.

> [!IMPORTANT]
> Un sistema LTI transforma la PSD multiplicando por $|H(\omega)|^2$. Después, la potencia se calcula integrando.

> [!WARNING]
> No multipliques la PSD por $H(\omega)$. La PSD siempre usa magnitud cuadrada.

> [!TIP]
> Para ruido blanco filtrado, la forma de la PSD de salida es básicamente la forma de $|H(\omega)|^2$.

## Preguntas tipo parcial

1. Calcule $S_y(\omega)$ dado $S_x(\omega)$ y $H(\omega)$.
2. Explique por qué aparece $|H(\omega)|^2$.
3. Calcule la potencia de salida de ruido blanco filtrado.
4. Explique cómo cambia la autocorrelación después de un pasa bajas.

## Mini-ejemplo

Si $S_n(\omega)=N_0/2$ y:

```math
H(\omega)=
\begin{cases}
1, & |\omega|<B\\
0, & |\omega|\geq B
\end{cases}
```

entonces:

```math
S_y(\omega)=
\begin{cases}
\dfrac{N_0}{2}, & |\omega|<B\\
0, & |\omega|\geq B
\end{cases}
```

y:

```math
P_y=
\frac{1}{2\pi}
\int_{-B}^{B}
\frac{N_0}{2}\,d\omega
=
\frac{N_0B}{2\pi}
```

---

# 8. Modulación DSB, canal AWGN y demodulación

## 8.1 Por qué se modula

Un mensaje $m(t)$ suele estar en banda base, cerca de frecuencia cero. Para transmitirlo por antena, canal pasabanda o multiplexación, se traslada a una frecuencia portadora $\omega_c$. Ese traslado se logra multiplicando por una señal sinusoidal.

La identidad clave es:

```math
\cos(\omega_ct)=\frac{1}{2}
\left(
e^{j\omega_ct}+e^{-j\omega_ct}
\right)
```

Multiplicar por $\cos(\omega_ct)$ genera dos copias espectrales: una desplazada a $+\omega_c$ y otra a $-\omega_c$.

## 8.2 DSB-SC

En DSB-SC:

```math
s(t)=A_cm(t)\cos(\omega_ct)
```

La portadora está suprimida porque no se transmite como un término independiente; solo aparece multiplicando al mensaje.

Si $M(\omega)$ es el espectro del mensaje:

```math
S(\omega)=
\frac{A_c}{2}
\left[
M(\omega-\omega_c)+M(\omega+\omega_c)
\right]
```

El ancho de banda es:

```math
B_{DSB}=2B_m
```

Esto ocurre porque el mensaje ocupa $B_m$ por encima y $B_m$ por debajo de la portadora.

En tiempo, la portadora rápida aparece con amplitud controlada por $m(t)$. En PSD, aparecen dos bandas laterales alrededor de $\pm\omega_c$.

## 8.3 AM convencional o DSB-LC

En AM convencional se transmite una portadora grande:

```math
s(t)=A_c[1+\mu m_n(t)]\cos(\omega_ct)
```

DSB-LC significa double sideband large carrier: dos bandas laterales más portadora grande.

$\mu$ es el índice de modulación. El mensaje se normaliza como $m_n(t)$ para controlar que:

```math
|m_n(t)|\leq1
```

La envolvente ideal es:

```math
A_c[1+\mu m_n(t)]
```

Para evitar sobremodulación:

```math
0\leq\mu\leq1
```

Si:

```math
\mu<1
```

hay submodulación. La envolvente sigue al mensaje sin tocar cero.

Si:

```math
\mu=1
```

hay modulación al límite. La envolvente puede tocar cero, pero no se invierte.

Si:

```math
\mu>1
```

hay sobremodulación. La envolvente cruza cero y se invierte. Un detector de envolvente recuperaría una señal distorsionada.

## 8.4 Canal AWGN

Un canal AWGN se modela como:

```math
y(t)=x(t)+n(t)
```

con:

```math
n(t)\sim AWGN
```

Si señal y ruido no están correlacionados:

```math
S_y(\omega)=S_x(\omega)+S_n(\omega)
```

En tiempo, el ruido se ve como fluctuaciones añadidas. En PSD, aparece como un piso espectral. En autocorrelación:

```math
R_y(\tau)=R_x(\tau)+R_n(\tau)
```

Un SNR alto permite distinguir la señal. Un SNR bajo hace que el piso de ruido tape componentes espectrales y distorsione la recuperación temporal.

## 8.5 Demodulación coherente

Para recuperar DSB-SC se multiplica otra vez por la portadora:

```math
y(t)\cos(\omega_ct)
=
A_cm(t)\cos^2(\omega_ct)
```

Usamos:

```math
\cos^2(\omega_ct)
=
\frac{1+\cos(2\omega_ct)}{2}
```

Entonces:

```math
y(t)\cos(\omega_ct)
=
\frac{A_c}{2}m(t)
+
\frac{A_c}{2}m(t)\cos(2\omega_ct)
```

La primera parte está en banda base:

```math
\frac{A_c}{2}m(t)
```

La segunda está alrededor de $2\omega_c$:

```math
\frac{A_c}{2}m(t)\cos(2\omega_ct)
```

Un filtro pasa bajas elimina la componente de alta frecuencia:

```math
\hat{m}(t)=
\frac{A_c}{2}m(t)
```

Luego se corrige la ganancia multiplicando por $2/A_c$.

La demodulación coherente necesita sincronización de frecuencia y fase. Si hay error de fase $\theta$, la señal recuperada se escala por un factor cercano a:

```math
\cos(\theta)
```

Si $\theta=90^\circ$, la recuperación ideal se anula.

## 8.6 Demodulación con Hilbert

La transformada de Hilbert genera una versión en cuadratura de la señal. La señal analítica es:

```math
x_a(t)=x(t)+j\mathcal{H}\{x(t)\}
```

La envolvente se obtiene como:

```math
A_x(t)=|x_a(t)|
```

La fase instantánea es:

```math
\phi_x(t)=\arg\{x_a(t)\}
```

Hilbert es útil cuando se quiere analizar envolvente y fase directamente, por ejemplo en señales AM o pasabanda. Puede ser sensible al ruido, a la sobremodulación, a cruces por cero y a señales que no están bien separadas espectralmente.

## 8.7 Comparación entre demodulación convencional y Hilbert

| Criterio | Filtro convencional | Hilbert |
|---|---|---|
| Recuperación | Muy buena si la portadora local está sincronizada | Buena si la señal analítica está bien definida |
| Ruido | El LPF puede rechazar ruido fuera de banda | La envolvente y fase pueden volverse ruidosas |
| Sincronización | Requiere fase y frecuencia correctas | No siempre requiere oscilador local explícito |
| Índice de modulación | La sobremodulación afecta fuertemente AM | También sufre con cruces de envolvente |
| Distorsión | Por error de fase, frecuencia o filtrado | Por ruido, mala banda o fase inestable |
| Complejidad | Conceptualmente directa | Requiere Hilbert y manejo de señal analítica |
| Interpretación | Recupera banda base | Da envolvente y fase instantánea |

> [!IMPORTANT]
> DSB traslada el espectro. Demodular coherentemente lo vuelve a bajar a banda base y elimina el término en $2\omega_c$.

> [!WARNING]
> DSB-SC no se recupera bien con detector de envolvente si no hay portadora.

> [!TIP]
> En demodulación coherente, siempre busca dos términos: uno útil en banda base y otro no deseado en $2\omega_c$.

## Preguntas tipo parcial

1. Demuestre el traslado espectral en DSB-SC.
2. Explique por qué $B_{DSB}=2B_m$.
3. Desarrolle la demodulación coherente paso a paso.
4. Explique qué ocurre cuando $\mu>1$ en AM.
5. Compare demodulación con filtro convencional y con Hilbert.

## Mini-ejemplo

Si:

```math
s(t)=A_cm(t)\cos(\omega_ct)
```

entonces:

```math
s(t)\cos(\omega_ct)
=
\frac{A_c}{2}m(t)
+
\frac{A_c}{2}m(t)\cos(2\omega_ct)
```

El pasa bajas conserva:

```math
\frac{A_c}{2}m(t)
```

Así se recupera el mensaje salvo por una ganancia constante.

---

# 9. PM: banda angosta, DSB y banda ancha

## 9.1 Qué significa modular la fase

En modulación en fase, la información no se coloca en la amplitud sino en el argumento del coseno:

```math
s_{PM}(t)=A_c\cos(\omega_ct+k_pm(t))
```

La fase instantánea es:

```math
\phi(t)=\omega_ct+k_pm(t)
```

$k_p$ mide cuántos radianes de fase se producen por unidad de mensaje. En PM ideal la amplitud $A_c$ permanece constante.

Si:

```math
m(t)=A_m\cos(\omega_mt)
```

entonces:

```math
s_{PM}(t)=A_c\cos(\omega_ct+\beta\cos(\omega_mt))
```

donde:

```math
\beta=k_pA_m
```

$\beta$ es el índice de fase. Mide la máxima desviación de fase.

## 9.2 PM de banda angosta

La condición de banda angosta es:

```math
\beta\ll1
```

Esto permite usar aproximaciones de ángulo pequeño:

```math
\cos(\beta m(t))\approx 1
```

```math
\sin(\beta m(t))\approx \beta m(t)
```

Partimos de:

```math
s_{PM}(t)=A_c\cos(\omega_ct+\beta m(t))
```

Usamos:

```math
\cos(a+b)=\cos(a)\cos(b)-\sin(a)\sin(b)
```

Entonces:

```math
s_{PM}(t)
=
A_c\cos(\omega_ct)\cos(\beta m(t))
-
A_c\sin(\omega_ct)\sin(\beta m(t))
```

Aplicando las aproximaciones:

```math
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
A_c\beta m(t)\sin(\omega_ct)
```

Esta expresión tiene dos partes. La primera es una portadora pura:

```math
A_c\cos(\omega_ct)
```

La segunda es proporcional a $m(t)$ multiplicado por $\sin(\omega_ct)$:

```math
-A_c\beta m(t)\sin(\omega_ct)
```

Eso es una modulación DSB-SC en cuadratura, porque usa seno en lugar de coseno.

## 9.3 Bandas laterales en NBPM

Para:

```math
m(t)=\cos(\omega_mt)
```

tenemos:

```math
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
A_c\beta\cos(\omega_mt)\sin(\omega_ct)
```

Usando:

```math
\sin(a)\cos(b)
=
\frac{1}{2}
\left[
\sin(a+b)+\sin(a-b)
\right]
```

queda:

```math
s_{PM}(t)
\approx
A_c\cos(\omega_ct)
-
\frac{A_c\beta}{2}
\left[
\sin((\omega_c+\omega_m)t)
+
\sin((\omega_c-\omega_m)t)
\right]
```

Aparecen principalmente:

```math
\omega_c-\omega_m
```

```math
\omega_c
```

```math
\omega_c+\omega_m
```

Por eso:

```math
B_{NBPM}\approx 2f_m
```

## 9.4 PM de banda ancha

Cuando:

```math
\beta>1
```

la aproximación de ángulo pequeño ya no es válida. Los términos de orden superior se vuelven importantes y aparecen muchas bandas laterales.

Para mensaje sinusoidal, el espectro contiene componentes en:

```math
\omega=\omega_c\pm n\omega_m
```

con:

```math
n=1,2,3,\dots
```

Teóricamente hay infinitas bandas laterales:

```math
B_{WBPM,\text{teórico}}\to\infty
```

En la práctica, las bandas muy lejanas tienen potencia muy pequeña y se ignoran.

## 9.5 Regla de Carson

La regla de Carson estima el ancho de banda práctico:

```math
B_{Carson}\approx 2(\Delta f+f_m)
```

Para PM sinusoidal:

```math
\Delta f=\beta f_m
```

Entonces:

```math
B_{Carson,PM}\approx 2(\beta+1)f_m
```

Carson no da el ancho de banda matemático total. Da una banda útil que contiene la mayor parte de la potencia.

> [!IMPORTANT]
> PM de banda angosta se parece a una portadora más DSB-SC en cuadratura. PM de banda ancha genera múltiples bandas laterales.

> [!WARNING]
> No uses $\sin(x)\approx x$ y $\cos(x)\approx1$ si $\beta$ no es mucho menor que 1.

> [!TIP]
> Para demostrar NBPM, empieza siempre con $\cos(a+b)$.

## Preguntas tipo parcial

1. Demuestre la aproximación de PM de banda angosta.
2. Explique por qué NBPM se relaciona con DSB-SC.
3. Explique por qué WBPM tiene ancho de banda teórico infinito.
4. Use Carson para estimar el ancho de banda de PM.

## Mini-ejemplo

Si $f_m=2\text{ kHz}$ y $\beta=0.2$, la señal puede tratarse como NBPM:

```math
B_{NBPM}\approx2f_m=4\text{ kHz}
```

Si $\beta=4$, se usa Carson:

```math
B_{Carson,PM}\approx2(\beta+1)f_m
```

```math
B_{Carson,PM}\approx2(5)(2\text{ kHz})=20\text{ kHz}
```

---

# Cómo se conectan todos los temas

Primero se define la señal aleatoria como un proceso con muchas realizaciones posibles. Como no se puede predecir exactamente cada valor, se calculan estadísticas: media, varianza, potencia, RMS y momentos.

Cuando esas estadísticas son estables en el tiempo, se puede usar el modelo WSS. En WSS, la autocorrelación depende solo del retardo $\tau$, lo que permite estudiar memoria temporal.

La autocorrelación no solo dice cuánto se parece la señal a sí misma; también contiene la potencia, porque:

```math
R_x(0)=P_x
```

Con Wiener-Khinchine, la autocorrelación se transforma en PSD:

```math
S_x(\omega)=\mathcal{F}\{R_x(\tau)\}
```

La PSD muestra cómo se reparte la potencia en frecuencia. Integrarla da la potencia total:

```math
P_x=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
```

Si la señal pasa por un sistema LTI, la PSD cambia por:

```math
S_y(\omega)=|H(\omega)|^2S_x(\omega)
```

Si aparece AWGN independiente, la PSD total suma señal más ruido:

```math
S_y(\omega)=S_x(\omega)+S_n(\omega)
```

En modulación DSB, multiplicar por una portadora traslada el espectro a $\pm\omega_c$. En demodulación coherente, multiplicar otra vez por la portadora devuelve una copia a banda base y otra a $2\omega_c$; el filtro pasa bajas conserva la primera.

En PM de banda angosta, la aproximación de ángulo pequeño convierte la señal en una portadora más un término DSB-SC en cuadratura. En PM de banda ancha, aparecen múltiples bandas laterales, y la regla de Carson estima el ancho de banda práctico.

> [!IMPORTANT]
> La cadena completa es: proceso aleatorio, estadísticas, WSS, autocorrelación, PSD, potencia, sistemas LTI, ruido, modulación, demodulación y PM.

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

# Fórmulas que debes dominar

Media:

```math
\mu_x(t)=E\{x(t)\}
```

Momento:

```math
m_n(t)=E\{x^n(t)\}
```

Varianza:

```math
\sigma_x^2(t)=E\{(x(t)-\mu_x(t))^2\}
```

Potencia:

```math
P_x=E\{|x(t)|^2\}
```

RMS:

```math
x_{RMS}=\sqrt{E\{x^2(t)\}}
```

WSS:

```math
E\{x(t)\}=\mu_x=\text{constante}
```

```math
R_x(t_1,t_2)=R_x(t_2-t_1)=R_x(\tau)
```

Autocorrelación:

```math
R_x(\tau)=E\{x(t)x^*(t+\tau)\}
```

Covarianza:

```math
K_x(\tau)=R_x(\tau)-|\mu_x|^2
```

Wiener-Khinchine:

```math
S_x(\omega)=\mathcal{F}\{R_x(\tau)\}
```

```math
R_x(\tau)=\mathcal{F}^{-1}\{S_x(\omega)\}
```

Potencia desde PSD:

```math
P_x=
\frac{1}{2\pi}
\int_{-\infty}^{\infty}S_x(\omega)\,d\omega
```

LTI:

```math
S_y(\omega)=|H(\omega)|^2S_x(\omega)
```

AWGN:

```math
y(t)=x(t)+n(t)
```

DSB-SC:

```math
s(t)=A_cm(t)\cos(\omega_ct)
```

AM:

```math
s(t)=A_c[1+\mu m_n(t)]\cos(\omega_ct)
```

PM:

```math
s_{PM}(t)=A_c\cos(\omega_ct+k_pm(t))
```

Carson para PM:

```math
B_{Carson,PM}\approx 2(\beta+1)f_m
```
