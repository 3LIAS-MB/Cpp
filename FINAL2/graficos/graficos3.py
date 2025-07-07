import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.ticker import PercentFormatter
import networkx as nx
import matplotlib
matplotlib.use('Agg')  # Modo no interactivo para guardar gráficos

# Configuración inicial mejorada
sns.set_theme(style="whitegrid", context="notebook", font_scale=1.1)
custom_palette = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B3"]
sns.set_palette(custom_palette)
plt.rcParams['figure.dpi'] = 120
plt.rcParams['savefig.dpi'] = 300
pd.set_option('display.max_columns', None)

# Carga de datos
def cargar_datos(archivo):
    df = pd.read_csv(archivo)
    if df.isnull().values.any():
        print("Advertencia: Los datos contienen valores nulos. Se llenarán con ceros.")
        df = df.fillna(0)

    # Calcular porcentajes evitando división por cero
    for col in ['S', 'I', 'R']:
        df[f'{col}_pct'] = df.groupby('Region')[col].transform(
            lambda x: (x / x.iloc[0] * 100) if x.iloc[0] != 0 else 0)
    
    df.replace([np.inf, -np.inf], np.nan, inplace=True)
    df.fillna(0, inplace=True)

    df['Total'] = df['S'] + df['I'] + df['R']
    return df

# Gráfico de evolución SIR mejorado
def grafico_evolucion(df, regiones=None, guardar=True):
    if regiones is None:
        regiones = sorted(df['Region'].unique())
    fig, axes = plt.subplots(len(regiones), 1, figsize=(12, 4*len(regiones)), sharex=True)
    if len(regiones) == 1:
        axes = [axes]
    for i, region in enumerate(regiones):
        data = df[df['Region'] == region]
        ax = axes[i]
        ax.stackplot(data['Dia'], data['S'], data['I'], data['R'],
                     labels=['Susceptibles', 'Infectados', 'Recuperados'],
                     colors=['#4C72B0', '#C44E52', '#55A868'],
                     alpha=0.8)
        peak_day = data.loc[data['I'].idxmax(), 'Dia']
        peak_value = data['I'].max()
        ax.axvline(peak_day, color='red', linestyle='--', alpha=0.7, linewidth=1.5)
        ax.text(peak_day+1, peak_value*0.8,
                f'Pico: Día {peak_day}\n{peak_value:.0f} infectados',
                bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
        ax.plot(data['Dia'], data['Total'], color='black', linestyle=':', linewidth=1.5, label='Población total')
        ax.set_title(f'Región {region} - Evolución SIR', pad=15)
        ax.set_ylabel('Población')
        ax.legend(loc='upper right', framealpha=1)
        ax.grid(True, alpha=0.3)
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
    plt.close()

# Gráfico de composición poblacional mejorado
def grafico_porcentajes(df, guardar=True):
    num_regions = len(df['Region'].unique())
    fig, axes = plt.subplots(1, num_regions, figsize=(6*num_regions, 6), sharey=True)
    if not isinstance(axes, np.ndarray):
        axes = np.array([axes])
    for i, region in enumerate(sorted(df['Region'].unique())):
        data = df[df['Region'] == region]
        axes[i].stackplot(data['Dia'], data['S_pct'], data['I_pct'], data['R_pct'],
                          labels=['Susceptibles', 'Infectados', 'Recuperados'],
                          colors=['#4C72B0', '#C44E52', '#55A868'],
                          alpha=0.8)
        axes[i].axhline(50, color='black', linestyle=':', alpha=0.5, linewidth=1)
        axes[i].axhline(25, color='black', linestyle=':', alpha=0.3, linewidth=0.5)
        axes[i].axhline(75, color='black', linestyle=':', alpha=0.3, linewidth=0.5)
        axes[i].set_title(f'Región {region} - Composición Poblacional', pad=15)
        axes[i].set_xlabel('Días')
        axes[i].legend(loc='lower center', bbox_to_anchor=(0.5, -0.25), ncol=3)
        axes[i].grid(True, alpha=0.3)
        final_stats = (f"Final:\nS: {data['S_pct'].iloc[-1]:.1f}%\n"
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
    plt.close()

# Gráfico de red de regiones
def grafico_red_regiones(archivo_vecinos, guardar=True):
    with open(archivo_vecinos, 'r') as f:
        lineas = [linea.strip() for linea in f if not linea.startswith('#') and linea.strip()]
    
    G = nx.Graph()
    for i, linea in enumerate(lineas):
        partes = linea.split()
        vecinos = list(map(int, partes[4:])) if len(partes) > 4 else []
        G.add_node(i, size=int(partes[0]))
        for v in vecinos:
            G.add_edge(i, v)
    
    pos = nx.spring_layout(G, seed=42)
    sizes = [G.nodes[n]['size'] / 1000 for n in G.nodes()]
    
    plt.figure(figsize=(10, 8))
    nx.draw_networkx_nodes(G, pos, node_size=sizes,
                           node_color='#4C72B0', alpha=0.9)
    nx.draw_networkx_edges(G, pos, width=1.5, alpha=0.5, edge_color='gray')
    nx.draw_networkx_labels(G, pos, font_size=12, font_weight='bold')
    
    plt.title('Red de Conexiones entre Regiones\n(Tamaño ≈ Población)', pad=20)
    plt.axis('off')
    
    min_size = min(sizes)
    max_size = max(sizes)
    for size in [min_size, (min_size + max_size)/2, max_size]:
        plt.scatter([], [], s=size, c='#4C72B0', alpha=0.8,
                    label=f'{int(size*1000):,} hab.')
    plt.legend(scatterpoints=1, frameon=True, labelspacing=2,
               title='Población', bbox_to_anchor=(1, 0.5), loc='center left')
    
    plt.tight_layout()
    if guardar:
        plt.savefig('red_regiones.png', bbox_inches='tight')
    plt.close()

# Función principal
def main():
    print("=== Visualización de Resultados SIR ===")
    archivo_csv = 'resultados_global.csv'
    archivo_vecinos = 'topologia.txt'
    try:
        df = cargar_datos(archivo_csv)
        print(f"Datos cargados correctamente: {len(df)} registros de {df['Region'].nunique()} regiones")
    except FileNotFoundError:
        print(f"Error: No se encontró el archivo {archivo_csv}")
        return

    print("Generando gráfico de evolución SIR...")
    grafico_evolucion(df)

    print("Generando gráfico de composición poblacional...")
    grafico_porcentajes(df)

    print("Generando gráfico de red de regiones...")
    try:
        grafico_red_regiones(archivo_vecinos)
    except FileNotFoundError:
        print(f"Advertencia: No se encontró el archivo {archivo_vecinos}, omitiendo red de regiones")

    print("¡Listo! Gráficos guardados como PNG.")

if __name__ == '__main__':
    main()
