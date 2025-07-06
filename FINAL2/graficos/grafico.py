import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import networkx as nx
from matplotlib.animation import FuncAnimation
import matplotlib
from matplotlib.colors import LinearSegmentedColormap

# Configuración profesional para paper académico
plt.style.use('seaborn-v0_8-whitegrid')
matplotlib.rcParams.update({
    'font.family': 'serif',
    'font.size': 12,
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'axes.titlesize': 14,
    'axes.labelsize': 12,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'legend.fontsize': 10
})

# Paleta de colores accesible
colors = ["#2b8cbe", "#e41a1c", "#4daf4a", "#984ea3", "#ff7f00", "#a65628"]
sns.set_palette(sns.color_palette(colors))

# --------------------------------------------------
# 1. FUNCIÓN PARA LEER Y PROCESAR LOS RESULTADOS
# --------------------------------------------------

def procesar_datos(archivo_csv):
    """Carga y procesa los datos epidemiológicos"""
    print(f"\nLeyendo archivo: {archivo_csv}")
    df = pd.read_csv(archivo_csv)
    
    # Imprimir información de verificación
    print(f"Primeras 3 filas del DataFrame:")
    print(df.head(3))
    print(f"Número total de filas: {len(df)}")
    print(f"Regiones únicas encontradas: {df['Region'].unique()}")
    
    # Resto del procesamiento
    df['Total'] = df['S'] + df['I'] + df['R']
    df['Porc_Infectados'] = (df['I'] / df['Total']) * 100
    df['Porc_Recuperados'] = (df['R'] / df['Total']) * 100
    
    picos = df.loc[df.groupby('Region')['I'].idxmax()]
    picos = picos[['Region', 'Dia', 'I']].rename(columns={'Dia': 'Dia_Pico', 'I': 'Max_Infectados'})
    
    return df, picos

# --------------------------------------------------
# 2. FUNCIONES DE VISUALIZACIÓN MEJORADAS
# --------------------------------------------------

def plot_curvas_sir(df, topologia):
    """Curvas SIR comparativas por región - Versión mejorada"""
    fig, axs = plt.subplots(3, 1, figsize=(10, 12), sharex=True)
    regiones = sorted(df['Region'].unique())
    
    # Crear paleta de colores única para cada región
    region_colors = plt.cm.tab10(np.linspace(0, 1, len(regiones)))
    
    for i, region in enumerate(regiones):
        region_df = df[df['Region'] == region]
        color = region_colors[i]
        
        # Susceptibles
        axs[0].plot(region_df['Dia'], region_df['S'], 
                   label=f'Región {region}', color=color, linewidth=1.5)
        
        # Infectados
        axs[1].plot(region_df['Dia'], region_df['I'], 
                   label=f'Región {region}', color=color, linewidth=1.5)
        
        # Recuperados
        axs[2].plot(region_df['Dia'], region_df['R'], 
                   label=f'Región {region}', color=color, linewidth=1.5)
    
    # Configurar ejes y títulos
    axs[0].set_title(f'Susceptibles - Topología {topologia}')
    axs[0].set_ylabel('Población')
    axs[0].grid(True, alpha=0.3)
    axs[0].legend(loc='upper right', fontsize=9)
    
    axs[1].set_title(f'Infectados - Topología {topologia}')
    axs[1].set_ylabel('Población')
    axs[1].grid(True, alpha=0.3)
    
    axs[2].set_title(f'Recuperados - Topología {topologia}')
    axs[2].set_ylabel('Población')
    axs[2].set_xlabel('Días')
    axs[2].grid(True, alpha=0.3)
    
    # Ajustar límites y formato
    for ax in axs:
        ax.set_ylim(bottom=0)
        ax.set_xlim(left=0)
    
    plt.tight_layout()
    plt.savefig(f'curvas_sir_{topologia}.png')
    plt.close()

def plot_evolucion_infectados(df, topologia):
    """Evolución de infectados por región - Reemplaza al heatmap"""
    plt.figure(figsize=(10, 6))
    
    # Crear paleta de colores única para cada región
    regiones = sorted(df['Region'].unique())
    region_colors = plt.cm.tab10(np.linspace(0, 1, len(regiones)))
    
    for i, region in enumerate(regiones):
        region_df = df[df['Region'] == region]
        plt.plot(region_df['Dia'], region_df['I'], 
                label=f'Región {region}', 
                color=region_colors[i],
                linewidth=2)
    
    plt.title(f'Evolución de Infectados - Topología {topologia}')
    plt.xlabel('Días')
    plt.ylabel('Población Infectada')
    plt.ylim(bottom=0)
    plt.legend(title='Región')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'evolucion_infectados_{topologia}.png')
    plt.close()

def plot_estado_final(df, picos, topologia):
    """Comparativa de estado final y picos epidémicos - Versión mejorada"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # Estado final (último día disponible)
    final_dia = df['Dia'].max()
    final = df[df['Dia'] == final_dia]
    regiones = sorted(final['Region'].unique())
    ancho = 0.25
    
    # Crear paleta de colores para los estados
    estado_colors = {
        'S': '#2b8cbe',  # Azul
        'I': '#e41a1c',  # Rojo
        'R': '#4daf4a'   # Verde
    }
    
    for i, estado in enumerate(['S', 'I', 'R']):
        # Obtener valores para todas las regiones
        valores = [final[final['Region'] == r][estado].values[0] for r in regiones]
        
        # Posiciones de las barras
        posiciones = np.arange(len(regiones)) + i * ancho
        
        ax1.bar(posiciones, valores, width=ancho, 
                label=estado, color=estado_colors[estado])
        
        # Agregar valores encima de las barras
        for j, valor in enumerate(valores):
            ax1.text(posiciones[j], valor + 0.5, f'{valor:.0f}', 
                    ha='center', va='bottom', fontsize=9)

    ax1.set_title(f'Estado Final (Día {final_dia}) - {topologia}')
    ax1.set_xticks(np.arange(len(regiones)) + ancho)
    ax1.set_xticklabels([f'Región {r}' for r in regiones])
    ax1.set_ylabel('Población')
    ax1.legend(title='Estado')
    ax1.grid(True, axis='y', alpha=0.3)
    
    # Día pico - Gráfico de puntos con tamaño proporcional
    ax2.scatter(picos['Region'], picos['Dia_Pico'], 
               s=picos['Max_Infectados']/50,  # Tamaño proporcional a infectados
               c=colors[0], alpha=0.7)
    
    # Etiquetar cada punto
    for i, row in picos.iterrows():
        ax2.text(row['Region'], row['Dia_Pico'] + 1, 
                f"{int(row['Max_Infectados'])}", 
                ha='center', fontsize=9)
    
    ax2.set_title(f'Día Pico de Infección - {topologia}')
    ax2.set_xlabel('Región')
    ax2.set_ylabel('Día del Pico')
    ax2.set_xticks(picos['Region'])
    ax2.set_xticklabels([f'Región {r}' for r in picos['Region']])
    ax2.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'estado_pico_{topologia}.png')
    plt.close()

def crear_red(topologia):
    """Crea gráfico de red para la topología"""
    G = nx.Graph()
    
    if topologia == 'anillo':
        # Regiones conectadas en anillo
        G.add_edges_from([(0,1), (1,2), (2,0)])
        pos = {0: (0,1), 1: (-1,0), 2: (1,0)}
    elif topologia == 'hub':
        # Hub central
        G.add_edges_from([(0,1), (0,2)])
        pos = {0: (0,0), 1: (-1,1), 2: (1,1)}
    elif topologia == 'completo':
        # Todas conectadas
        G.add_edges_from([(0,1), (0,2), (1,2)])
        pos = {0: (0,1), 1: (-1,0), 2: (1,0)}
    else:  # aislado
        G.add_nodes_from([0, 1, 2])  # Nodos sin conexiones
        pos = {0: (0,1), 1: (-1,0), 2: (1,0)}
    
    return G, pos

def plot_red_epidemiologica(df, topologia, dia):
    """Diagrama de red con estado epidemiológico - Versión mejorada"""
    G, pos = crear_red(topologia)
    estado_dia = df[df['Dia'] == dia]
    
    # Si no hay datos para ese día, usar el último día disponible
    if estado_dia.empty:
        dia = df['Dia'].max()
        estado_dia = df[df['Dia'] == dia]
    
    # Asignar tamaños y colores según infectados
    sizes = []
    node_colors = []
    max_infectados = estado_dia['I'].max() if not estado_dia.empty else 1
    
    # Paleta de colores para los nodos
    cmap = plt.cm.Reds
    
    for region in G.nodes():
        region_data = estado_dia[estado_dia['Region'] == region]
        if not region_data.empty:
            infectados = region_data['I'].values[0]
            # Tamaño proporcional a infectados
            size = 800 + (infectados / max_infectados) * 2000
            sizes.append(size)
            # Color proporcional a porcentaje de infectados
            porcentaje = (infectados / region_data['Total'].values[0]) * 100
            node_colors.append(cmap(porcentaje / 100))
        else:
            sizes.append(800)
            node_colors.append(cmap(0))  # Color mínimo
    
    plt.figure(figsize=(10, 8))
    
    # Dibujar la red
    nx.draw_networkx_nodes(
        G, pos, 
        node_size=sizes, 
        node_color=node_colors,
        edgecolors='black',
        linewidths=1.5
    )
    
    # Grosor de las aristas proporcional a la conectividad
    edge_widths = [3.0 for edge in G.edges()]
    nx.draw_networkx_edges(G, pos, width=edge_widths, alpha=0.7)
    
    # Etiquetas de los nodos
    nx.draw_networkx_labels(
        G, pos, 
        font_size=14, 
        font_weight='bold',
        font_color='black'
    )
    
    # Barra de color para porcentaje de infectados
    sm = plt.cm.ScalarMappable(
        cmap=cmap, 
        norm=plt.Normalize(vmin=0, vmax=100)
    )
    sm.set_array([])
    cbar = plt.colorbar(sm, label='% Infectados', shrink=0.8)
    
    plt.title(f'Estado Epidemiológico - {topologia}\nDía {dia}', fontsize=16)
    plt.axis('off')
    plt.tight_layout()
    plt.savefig(f'red_{topologia}_dia{dia}.png')
    plt.close()
    
def crear_animacion(df, topologia):
    """Animación de propagación epidémica - Versión mejorada"""
    fig, ax = plt.subplots(figsize=(10, 6))
    max_dia = df['Dia'].max()
    
    # Calcular valores máximos para escalas consistentes
    max_infectados = df['I'].max()
    max_region = df['Region'].max()
    
    def update(frame):
        ax.clear()
        dia_data = df[df['Dia'] == frame]
        
        if dia_data.empty:
            return
        
        # Barras para infectados
        bars = ax.bar(
            dia_data['Region'], 
            dia_data['I'], 
            color='#e41a1c',  # Rojo
            alpha=0.8,
            label='Infectados'
        )
        
        # Etiquetas con valores en las barras
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                    f'{int(height)}', 
                    ha='center', va='bottom', fontsize=9)
        
        # Línea para porcentaje de infectados
        ax2 = ax.twinx()
        line = ax2.plot(
            dia_data['Region'], 
            dia_data['Porc_Infectados'], 
            'o-', color='#984ea3',  # Púrpura
            linewidth=2,
            markersize=6,
            label='% Infectados'
        )
        
        ax.set_title(f'Propagación Epidémica - {topologia}\nDía {frame}', fontsize=14)
        ax.set_xlabel('Región')
        ax.set_ylabel('Infectados')
        ax.set_ylim(0, max_infectados * 1.1)
        ax.set_xlim(-0.5, max_region + 0.5)
        ax.set_xticks(dia_data['Region'])
        ax.set_xticklabels([f'Región {r}' for r in dia_data['Region']])
        ax.grid(True, alpha=0.3)
        
        ax2.set_ylabel('% Población Infectada', color='#984ea3')
        ax2.set_ylim(0, 100)
        ax2.tick_params(axis='y', labelcolor='#984ea3')
        
        # Combinar leyendas
        lines, labels = ax.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax2.legend(lines + lines2, labels + labels2, loc='upper left')
    
    anim = FuncAnimation(fig, update, frames=range(0, max_dia + 1, 5), interval=400)
    anim.save(f'propagacion_{topologia}.gif', writer='pillow')
    plt.close()
    return anim

def calcular_metricas(df, picos):
    """Calcula métricas epidemiológicas clave"""
    # Calcular nuevos infectados
    df['nuevos_infectados'] = df.groupby('Region')['I'].diff().fillna(0)
    
    # Calcular R_efectivo evitando división por cero
    with np.errstate(divide='ignore', invalid='ignore'):
        df['R_efectivo'] = np.where(
            (df['S'] > 0) & (df['I'] > 0),
            (df['nuevos_infectados'] / df['S']) * (df['Total'] / df['I']),
            0
        )
    
    # Reemplazar infinitos/NaN y calcular promedio
    df['R_efectivo'] = df['R_efectivo'].replace([np.inf, -np.inf], np.nan)
    R0 = df['R_efectivo'].mean(skipna=True)
    
    # Estado final (último día)
    final_dia = df['Dia'].max()
    estado_final = df[df['Dia'] == final_dia]
    
    # Resumen estadístico
    resumen = {
        'R0_efectivo': R0 if not np.isnan(R0) else 0,
        'max_infectados': picos['Max_Infectados'].max(),
        'dia_pico_promedio': picos['Dia_Pico'].mean(),
        'recuperados_finales': estado_final['R'].mean(),
        'tiempo_epidemia': picos['Dia_Pico'].max() - picos['Dia_Pico'].min()
    }
    
    return resumen

# --------------------------------------------------
# 3. EJECUCIÓN PRINCIPAL
# --------------------------------------------------

def main():
    # Configuración de topologías
    topologias = {
        'aislado': 'resultados_aislado.csv',
        'anillo': 'resultados_anillo.csv',
        'hub': 'resultados_hub.csv',
        'completo': 'resultados_completo.csv'
    }
    
    # Procesar y visualizar cada topología
    for topo, archivo in topologias.items():
        print(f"\n{'='*50}")
        print(f"Procesando topología: {topo}")
        print(f"{'='*50}")
        
        try:
            # Verificar que el archivo existe y su tamaño
            import os
            if os.path.exists(archivo):
                print(f"Tamaño del archivo {archivo}: {os.path.getsize(archivo)} bytes")
            else:
                print(f"¡ADVERTENCIA! Archivo no encontrado: {archivo}")
                continue

            # Cargar datos
            df, picos = procesar_datos(archivo)
            
            # Verificar datos antes de graficar
            print("\nEstadísticas del DataFrame:")
            print(f"Rango de días: {df['Dia'].min()} a {df['Dia'].max()}")
            print(f"Máximo de infectados por región:")
            for region in df['Region'].unique():
                max_inf = df[df['Region'] == region]['I'].max()
                print(f"Región {region}: {max_inf}")

            # Generar visualizaciones
            print("\nGenerando gráficos...")
            plot_curvas_sir(df, topo)
            plot_evolucion_infectados(df, topo)  # Reemplazo del heatmap
            plot_estado_final(df, picos, topo)
            
            dia_red = int(picos['Dia_Pico'].mean())
            plot_red_epidemiologica(df, topo, dia_red)
            
            # Calcular y mostrar métricas
            metricas = calcular_metricas(df, picos)
            print("\nMétricas calculadas:")
            for k, v in metricas.items():
                print(f"  - {k}: {v:.4f}")
                
        except Exception as e:
            import traceback
            print(f"\n¡Error procesando {topo}!")
            print(f"Error: {str(e)}")
            print("\nStack trace:")
            print(traceback.format_exc())
    
    print("\nProceso completado. Gráficos guardados en el directorio actual.")

if __name__ == "__main__":
    main()