import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.ticker import FuncFormatter
import numpy as np

# Configuración inicial
sns.set_theme(style="whitegrid")  # Reemplazado plt.style.use('seaborn')
sns.set_palette("husl")
pd.set_option('display.max_columns', None)

# 1. Cargar los datos
def cargar_datos(archivo):
    df = pd.read_csv(archivo)
    
    # Calcular porcentajes
    for col in ['S', 'I', 'R']:
        df[f'{col}_pct'] = df.groupby('Region')[col].transform(
            lambda x: x / x.iloc[0] * 100)
    
    return df

# 2. Gráfico de evolución temporal por región
def grafico_evolucion(df, regiones=None):
    if regiones is None:
        regiones = df['Region'].unique()
    
    fig, axes = plt.subplots(len(regiones), 1, 
                           figsize=(14, 6*len(regiones)),
                           sharex=True)
    
    if len(regiones) == 1:
        axes = [axes]
    
    for i, region in enumerate(regiones):
        data = df[df['Region'] == region]
        
        ax = axes[i]
        ax.plot(data['Dia'], data['S'], label='Susceptibles', linewidth=2)
        ax.plot(data['Dia'], data['I'], label='Infectados', linewidth=2)
        ax.plot(data['Dia'], data['R'], label='Recuperados', linewidth=2)
        
        # Marcar el pico de infección
        peak_day = data.loc[data['I'].idxmax(), 'Dia']
        peak_value = data['I'].max()
        ax.axvline(peak_day, color='red', linestyle='--', alpha=0.5)
        ax.annotate(f'Pico: Día {peak_day}\n{peak_value:.0f} infectados',
                   xy=(peak_day, peak_value),
                   xytext=(10, 10), textcoords='offset points',
                   bbox=dict(boxstyle='round,pad=0.5', fc='white', alpha=0.8),
                   arrowprops=dict(arrowstyle='->'))
        
        ax.set_title(f'Región {region} - Evolución SIR')
        ax.set_ylabel('Población')
        ax.legend()
        ax.grid(True, alpha=0.3)
    
    plt.xlabel('Días')
    plt.tight_layout()
    plt.savefig('evolucion_sir_por_region.png', dpi=300)
    plt.show()

# 3. Gráfico comparativo de infectados
def grafico_comparativo_infectados(df):
    plt.figure(figsize=(14, 7))
    
    for region in df['Region'].unique():
        data = df[df['Region'] == region]
        plt.plot(data['Dia'], data['I'], 
                label=f'Región {region}', 
                linewidth=2.5)
    
    plt.title('Comparación de Infectados entre Regiones')
    plt.xlabel('Días')
    plt.ylabel('Número de Infectados')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('comparacion_infectados.png', dpi=300)
    plt.show()

# 4. Gráfico de porcentajes acumulados
def grafico_porcentajes(df):
    fig, axes = plt.subplots(1, 3, figsize=(18, 6), sharey=True)
    
    for i, region in enumerate(df['Region'].unique()):
        data = df[df['Region'] == region]
        
        axes[i].stackplot(data['Dia'], 
                         data['S_pct'], 
                         data['I_pct'], 
                         data['R_pct'],
                         labels=['Susceptibles', 'Infectados', 'Recuperados'],
                         alpha=0.7)
        
        axes[i].set_title(f'Región {region} - Composición Poblacional')
        axes[i].set_xlabel('Días')
        axes[i].legend(loc='lower center')
        axes[i].grid(True, alpha=0.3)
    
    axes[0].set_ylabel('Porcentaje de Población')
    plt.tight_layout()
    plt.savefig('composicion_poblacional.png', dpi=300)
    plt.show()

# 5. Gráfico de pequeñas múltiples (Small Multiples)
def small_multiples(df):
    g = sns.FacetGrid(df, col='Region', hue='Region', 
                     col_wrap=3, height=4, aspect=1.5)
    
    g.map(plt.plot, 'Dia', 'S', label='Susceptibles')
    g.map(plt.plot, 'Dia', 'I', label='Infectados')
    g.map(plt.plot, 'Dia', 'R', label='Recuperados')
    
    g.set_titles('Región {col_name}')
    g.set_axis_labels('Días', 'Población')
    g.add_legend()
    
    plt.tight_layout()
    plt.savefig('small_multiples_sir.png', dpi=300)
    plt.show()

# Función principal
def main():
    # Cargar datos (asegúrate de que el archivo esté en el mismo directorio)
    archivo_csv = 'resultados_global.csv'
    df = cargar_datos(archivo_csv)
    
    # Generar todos los gráficos
    grafico_evolucion(df)
    grafico_comparativo_infectados(df)
    grafico_porcentajes(df)
    small_multiples(df)
    
    print("¡Gráficos generados con éxito! Busca los archivos:")
    print("- evolucion_sir_por_region.png")
    print("- comparacion_infectados.png")
    print("- composicion_poblacional.png")
    print("- small_multiples_sir.png")

if __name__ == '__main__':
    main()