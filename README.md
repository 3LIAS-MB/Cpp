<!-- mpicc fractal_mpi.c -o julia -lm -std=c99 -->

Soy un estudiante universitario y estoy desarrollando un **proyecto de Programación Paralela** para mi curso de "Programación III". El objetivo es simular la propagación del cáncer, un tema que nuestro profesor ha destacado por su conexión con los patrones fractales. Necesito generar un **código C completo, robusto, bien estructurado y documentado** utilizando MPI (MPICH o OpenMPI) para esta simulación.

El proyecto se basa en la observación de que algunos tipos de cáncer presentan patrones de crecimiento similares a los fractales, lo que permite modelar su expansión. Nuestro profesor sugirió que este tipo de crecimiento ramificado puede ser simulado utilizando el modelo de **Diffusion-Limited Aggregation (DLA)**.

A partir de esta información, por favor, genera un programa en lenguaje C con MPI que cumpla con los siguientes requisitos:

🧠 **Requisitos conceptuales:**
*   El modelo de **Diffusion-Limited Aggregation (DLA)** debe ser la base fractal para simular el crecimiento del tumor. Esto implica que las nuevas células se unen al "tumor" existente siguiendo una lógica de difusión y agregación.
*   La simulación debe representarse en una **malla 2D** (una cuadrícula discreta) donde cada celda puede estar vacía o contener una célula cancerígena.
*   Las células nuevas pueden aparecer y crecer si están en contacto (vecindad de Moore, 8 direcciones) con células cancerígenas ya activas, según una **probabilidad de división celular**. Además, las células existentes pueden migrar o morir según sus respectivas probabilidades.

⚙️ **Requisitos técnicos específicos para la generación de código:**
*   **Lenguaje de programación:** C (estándar C99 o posterior).
*   **Paralelización:** MPI (utilizando librerías como MPICH o OpenMPI). El código debe ser diseñado para **ejecutarse eficientemente en múltiples procesos MPI**.
*   **Dimensiones de la malla de simulación:** 512x512 celdas.
*   **Semilla inicial del tumor:** Una única célula cancerígena ubicada precisamente en el **centro** de la malla al inicio de la simulación.
*   **Probabilidad de división celular:** 0.75 (la probabilidad de que una nueva célula aparezca en una celda vacía adyacente a una célula activa).
*   **Probabilidad de migración:** 0.25 (la probabilidad de que una célula cancerígena se mueva a una celda vacía adyacente).
*   **Probabilidad de muerte:** 0.05 (la probabilidad de que una célula cancerígena existente muera y la celda quede vacía).
*   **Número total de iteraciones:** 1000 pasos de tiempo para la simulación.
*   **Descomposición de la malla:** Implementar una **descomposición de dominio en bloques 2D (grid decomposition)** entre los procesos MPI para distribuir la carga de trabajo y optimizar la comunicación de vecinos.
*   **Comunicación entre procesos:** Es crucial la **sincronización en cada paso de tiempo** para el intercambio de información de las celdas en los bordes de los subdominios (ghost cells/halo exchange).
*   El código debe incluir la **inicialización y finalización adecuada de MPI**.

📊 **Salida esperada y visualización:**
*   Generar **imágenes de la malla** en formato **PGM** (Portable Graymap) cada **100 iteraciones** para visualizar el crecimiento del tumor. La imagen debe representar el estado de la malla (por ejemplo, blanco para celdas vacías, negro para celdas con cáncer).
*   **(Opcional, pero recomendado)** Calcular y reportar **métricas simples** como la cantidad total de células vivas en cada iteración, imprimirlas en consola o en un archivo de texto.

🎯 **Objetivo final:**
El objetivo es obtener una **simulación funcional, visualmente representativa y computacionalmente eficiente** del crecimiento tumoral basada en principios fractales y ejecutable de forma paralela en múltiples procesos MPI. El código debe ser **fácil de compilar y ejecutar**, e incluir **comentarios explicativos** para cada sección importante.
Justificación de los cambios:
1.
Contexto para la IA: Se comienza el prompt contextualizando que es un "proyecto universitario" y se pide "código C completo, robusto, bien estructurado y documentado". Esto ayuda a la IA a entender el nivel de calidad y completitud esperado, evitando generar solo fragmentos.
2.
Énfasis en MPI: Se subraya que el código debe ser "diseñado para ejecutarse eficientemente en múltiples procesos MPI", reforzando la importancia de la paralelización.
3.
Claridad en DLA: Se explica brevemente qué implica DLA en este contexto ("nuevas células se unen al 'tumor' existente siguiendo una lógica de difusión y agregación") para asegurar que la IA implemente el modelo correctamente según la interpretación del problema.
4.
Movimiento y Muerte: Se añade un pequeño matiz a la aparición de células: "además, las células existentes pueden migrar o morir según sus respectivas probabilidades". Esto refuerza los requisitos ya dados y los integra mejor en la descripción conceptual.
5.
Comunicación Explícita: Aunque ya lo mencionó, especificar "ghost cells/halo exchange" en la comunicación es un detalle técnico que una IA experta en MPI entenderá muy bien y que garantiza una implementación correcta de la descomposición de dominio.
6.
Formato de Salida PGM: Se añade una pequeña aclaración sobre cómo se deben representar las celdas en la imagen PGM (ej. blanco/negro) para guiar a la IA en la generación de imágenes claras.
7.
Conclusividad: El objetivo final se reformula para sonar más formal y alineado con los objetivos de un trabajo universitario: "simulación funcional, visualmente representativa y computacionalmente eficiente".
