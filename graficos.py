import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.ticker import PercentFormatter
import networkx as nx
from matplotlib.colors import LinearSegmentedColormap

# Configuración inicial mejorada
sns.set_theme(style="whitegrid", context="notebook", font_scale=1.1)
custom_palette = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B3"]
sns.set_palette(custom_palette)
plt.rcParams['figure.dpi'] = 120
plt.rcParams['savefig.dpi'] = 300
pd.set_option('display.max_columns', None)

# 1. Función mejorada para cargar datos
def cargar_datos(archivo):
    """Carga y prepara los datos de la simulación"""
    df = pd.read_csv(archivo)
    
    # Verificar y limpiar datos
    if df.isnull().values.any():
        print("Advertencia: Los datos contienen valores nulos. Se llenarán con ceros.")
        df = df.fillna(0)
    
    # Calcular métricas adicionales
    for col in ['S', 'I', 'R']:
        df[f'{col}_pct'] = df.groupby('Region')[col].transform(
            lambda x: x / x.iloc[0] * 100)
    
    # Calcular el total diario
    df['Total'] = df['S'] + df['I'] + df['R']
    
    return df

# 2. Gráfico de evolución temporal mejorado
def grafico_evolucion(df, regiones=None, guardar=True):
    """Muestra la evolución temporal para cada región"""
    if regiones is None:
        regiones = sorted(df['Region'].unique())
    
    fig, axes = plt.subplots(len(regiones), 1, 
                           figsize=(12, 4*len(regiones)),
                           sharex=True)
    
    if len(regiones) == 1:
        axes = [axes]
    
    for i, region in enumerate(regiones):
        data = df[df['Region'] == region]
        
        ax = axes[i]
        # Gráfico de áreas apiladas
        ax.stackplot(data['Dia'], 
                    data['S'], data['I'], data['R'],
                    labels=['Susceptibles', 'Infectados', 'Recuperados'],
                    colors=['#4C72B0', '#C44E52', '#55A868'],
                    alpha=0.8)
        
        # Marcar el pico de infección
        peak_day = data.loc[data['I'].idxmax(), 'Dia']
        peak_value = data['I'].max()
        ax.axvline(peak_day, color='red', linestyle='--', alpha=0.7, linewidth=1.5)
        ax.text(peak_day+1, peak_value*0.8, 
               f'Pico: Día {peak_day}\n{peak_value:.0f} infectados',
               bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
        
        # Añadir línea de población total
        ax.plot(data['Dia'], data['Total'], 
               color='black', linestyle=':', linewidth=1.5,
               label='Población total')
        
        ax.set_title(f'Región {region} - Evolución SIR', pad=15)
        ax.set_ylabel('Población')
        ax.legend(loc='upper right', framealpha=1)
        ax.grid(True, alpha=0.3)
        
        # Añadir cuadro de estadísticas
        stats_text = (f"Población inicial: {data['Total'].iloc[0]:,.0f}\n"
                     f"Máx. infectados: {peak_value:,.0f} ({peak_value/data['Total'].iloc[0]:.1%})\n"
                     f"Días con >1% infectados: {(data['I']/data['Total'] > 0.01).sum()}")
        ax.text(0.98, 0.55, stats_text,
               transform=ax.transAxes,
               verticalalignment='top', horizontalalignment='right',
               bbox=dict(facecolor='white', alpha=0.8))
    
    plt.xlabel('Días')
    plt.tight_layout()
    if guardar:
        plt.savefig('evolucion_sir_mejorado.png', bbox_inches='tight')
    plt.show()

# 3. Gráfico comparativo mejorado
def grafico_comparativo_infectados(df, guardar=True):
    """Comparación mejorada de infectados entre regiones"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10), sharex=True)
    
    # Gráfico lineal normal
    for region in df['Region'].unique():
        data = df[df['Region'] == region]
        ax1.plot(data['Dia'], data['I'], 
                label=f'Región {region}', 
                linewidth=2.5)
    
    ax1.set_title('Comparación de Infectados entre Regiones (Escala Lineal)')
    ax1.set_ylabel('Número de Infectados')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Gráfico logarítmico
    for region in df['Region'].unique():
        data = df[df['Region'] == region]
        ax2.plot(data['Dia'], data['I'], 
                label=f'Región {region}', 
                linewidth=2.5)
    
    ax2.set_title('Comparación de Infectados entre Regiones (Escala Logarítmica)')
    ax2.set_ylabel('Número de Infectados (log)')
    ax2.set_yscale('log')
    ax2.grid(True, alpha=0.3, which='both')
    
    plt.xlabel('Días')
    plt.tight_layout()
    if guardar:
        plt.savefig('comparacion_infectados_mejorado.png', bbox_inches='tight')
    plt.show()

# 4. Gráfico de porcentajes mejorado
def grafico_porcentajes(df, guardar=True):
    """Muestra la composición porcentual de la población"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 6), sharey=True)
    
    for i, region in enumerate(sorted(df['Region'].unique())):
        data = df[df['Region'] == region]
        
        # Gráfico de áreas
        axes[i].stackplot(data['Dia'], 
                         data['S_pct'], 
                         data['I_pct'], 
                         data['R_pct'],
                         labels=['Susceptibles', 'Infectados', 'Recuperados'],
                         colors=['#4C72B0', '#C44E52', '#55A868'],
                         alpha=0.8)
        
        # Líneas de referencia
        axes[i].axhline(50, color='black', linestyle=':', alpha=0.5, linewidth=1)
        axes[i].axhline(25, color='black', linestyle=':', alpha=0.3, linewidth=0.5)
        axes[i].axhline(75, color='black', linestyle=':', alpha=0.3, linewidth=0.5)
        
        axes[i].set_title(f'Región {region} - Composición Poblacional', pad=15)
        axes[i].set_xlabel('Días')
        axes[i].legend(loc='lower center', bbox_to_anchor=(0.5, -0.25), ncol=3)
        axes[i].grid(True, alpha=0.3)
        
        # Añadir cuadro con estadísticas finales
        final_stats = (f"Final:\n"
                      f"S: {data['S_pct'].iloc[-1]:.1f}%\n"
                      f"I: {data['I_pct'].iloc[-1]:.1f}%\n"
                      f"R: {data['R_pct'].iloc[-1]:.1f}%")
        axes[i].text(0.98, 0.98, final_stats,
                   transform=axes[i].transAxes,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))
    
    axes[0].set_ylabel('Porcentaje de Población')
    axes[0].yaxis.set_major_formatter(PercentFormatter())
    plt.tight_layout()
    if guardar:
        plt.savefig('composicion_poblacional_mejorado.png', bbox_inches='tight')
    plt.show()

# 5. Gráfico de red de regiones (nuevo)
def grafico_red_regiones(archivo_vecinos, guardar=True):
    """Visualiza las conexiones entre regiones"""
    # Leer archivo de vecinos
    with open(archivo_vecinos, 'r') as f:
        lineas = [linea.strip() for linea in f if not linea.startswith('#') and linea.strip()]
    
    # Crear grafo
    G = nx.Graph()
    for i, linea in enumerate(lineas):
        partes = linea.split()
        vecinos = list(map(int, partes[4:])) if len(partes) > 4 else []
        G.add_node(i, size=int(partes[0]))
        for v in vecinos:
            G.add_edge(i, v)
    
    # Calcular posiciones
    pos = nx.spring_layout(G, seed=42)
    
    # Tamaño de nodos proporcional a población
    sizes = [G.nodes[n]['size']/1000 for n in G.nodes()]
    
    plt.figure(figsize=(10, 8))
    nx.draw_networkx_nodes(G, pos, node_size=sizes, 
                          node_color='#4C72B0', alpha=0.9)
    nx.draw_networkx_edges(G, pos, width=1.5, alpha=0.5, edge_color='gray')
    nx.draw_networkx_labels(G, pos, font_size=12, font_weight='bold')
    
    plt.title('Red de Conexiones entre Regiones\n(Tamaño ≈ Población)', pad=20)
    plt.axis('off')
    
    # Añadir leyenda de tamaño
    min_size = min(sizes)
    max_size = max(sizes)
    for size in [min_size, (min_size+max_size)/2, max_size]:
        plt.scatter([], [], s=size, c='#4C72B0', alpha=0.8,
                   label=f'{int(size*1000):,} hab.')
    plt.legend(scatterpoints=1, frameon=True, labelspacing=2,
              title='Población', bbox_to_anchor=(1, 0.5), loc='center left')
    
    plt.tight_layout()
    if guardar:
        plt.savefig('red_regiones.png', bbox_inches='tight')
    plt.show()

# 6. Heatmap de correlación (nuevo)
def heatmap_correlacion(df, guardar=True):
    """Muestra correlaciones entre brotes en diferentes regiones"""
    pivot = df.pivot_table(index='Dia', columns='Region', values='I')
    correlaciones = pivot.corr()
    
    # Crear máscara para el triángulo superior
    mask = np.triu(np.ones_like(correlaciones, dtype=bool))
    
    plt.figure(figsize=(10, 8))
    cmap = LinearSegmentedColormap.from_list('custom_cmap', ['#4C72B0', 'white', '#C44E52'])
    
    sns.heatmap(correlaciones, mask=mask, annot=True, fmt=".2f",
               cmap=cmap, center=0, vmin=-1, vmax=1,
               square=True, linewidths=0.5, cbar_kws={"shrink": 0.8})
    
    plt.title('Correlación entre Brotes en Diferentes Regiones\n', pad=20)
    plt.tight_layout()
    if guardar:
        plt.savefig('correlacion_regiones.png', bbox_inches='tight')
    plt.show()

# Función principal mejorada
def main():
    print("=== Visualización de Resultados de Simulación SIR con MPI ===")
    
    # Cargar datos
    archivo_csv = 'resultados_global.csv'
    try:
        df = cargar_datos(archivo_csv)
        print(f"Datos cargados correctamente: {len(df)} registros de {df['Region'].nunique()} regiones")
    except FileNotFoundError:
        print(f"Error: No se encontró el archivo {archivo_csv}")
        print("Asegúrese de que el archivo CSV está en el mismo directorio")
        return
    
    # Generar gráficos mejorados
    print("\nGenerando gráficos...")
    grafico_evolucion(df)
    grafico_comparativo_infectados(df)
    grafico_porcentajes(df)
    
    # Generar gráficos nuevos (requieren archivo de vecinos)
    try:
        grafico_red_regiones('vecinosTodos.txt')
    except FileNotFoundError:
        print("\nAdvertencia: No se encontró 'vecinosTodos.txt' - omitiendo gráfico de red")
    
    heatmap_correlacion(df)
    
    print("\n¡Gráficos generados con éxito! Archivos creados:")
    print("- evolucion_sir_mejorado.png")
    print("- comparacion_infectados_mejorado.png")
    print("- composicion_poblacional_mejorado.png")
    print("- red_regiones.png (si se proporcionó archivo de vecinos)")
    print("- correlacion_regiones.png")

if __name__ == '__main__':
    main()
