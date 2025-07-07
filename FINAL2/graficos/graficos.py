import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib

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
    df = pd.read_csv(archivo_csv)
    
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
    # Configuración de topologías (solo aislado, anillo y completo)
    topologias = {
        'aislado': 'resultados_aislado.csv',
        'anillo': 'resultados_anillo.csv',
        'completo': 'resultados_completo.csv'
    }
    
    # Procesar y visualizar cada topología
    for topo, archivo in topologias.items():
        print(f"\nProcesando topología: {topo}")
        
        try:
            # Verificar que el archivo existe
            import os
            if not os.path.exists(archivo):
                print(f"¡ADVERTENCIA! Archivo no encontrado: {archivo}")
                continue

            # Cargar datos
            df, picos = procesar_datos(archivo)
            
            # Generar visualizaciones
            plot_curvas_sir(df, topo)
            plot_evolucion_infectados(df, topo)
            
            # Calcular métricas
            metricas = calcular_metricas(df, picos)
            
            print(f"Topología {topo} procesada correctamente. Gráficos generados.")
                
        except Exception as e:
            print(f"Error procesando {topo}: {str(e)}")
            continue
    
    print("\nProceso completado. Todos los gráficos guardados en el directorio actual.")

if __name__ == "__main__":
    main()