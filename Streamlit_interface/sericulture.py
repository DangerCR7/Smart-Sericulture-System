import streamlit as st
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
import random

# Load the dataset
file_path = 'sericulture_dataset.csv'

try:
    data = pd.read_csv(file_path)
except FileNotFoundError:
    st.error("File not found. Please ensure 'sericulture_dataset.csv' is in the correct directory.")
    st.stop()
except Exception as e:
    st.error(f"An error occurred while loading the dataset: {e}")
    st.stop()

# Check if the required columns are in the dataset
required_columns = ['Temperature (°C)', 'Humidity (%)', 'Light Intensity (Lux)', 
                    'Feeding Quality', 'Cocoon Yield (kg)']
if not all(column in data.columns for column in required_columns):
    st.error(f"The dataset is missing one or more required columns: {', '.join(required_columns)}")
    st.stop()

# Encode 'Health Status' if present
if 'Health Status' in data.columns:
    label_encoder = LabelEncoder()
    data['Health Status'] = label_encoder.fit_transform(data['Health Status'])

# Define features and target variable
features = ['Temperature (°C)', 'Humidity (%)', 'Light Intensity (Lux)', 'Feeding Quality']
target = 'Cocoon Yield (kg)'

X = data[features]
y = data[target]

# Split into training and test sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train the Random Forest model
model = RandomForestRegressor(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Streamlit App
st.title("Sericulture Monitoring and Cocoon Yield Prediction")

# Sidebar for feeding quality input
st.sidebar.header("Input Parameters")
feeding_quality = st.sidebar.selectbox("Feeding Quality (1 to 5)", [1, 2, 3, 4, 5])

# Real-time sensor simulation
st.subheader("Real-Time Sensor Monitoring")
sensor_placeholder = st.empty()

# Function to simulate sensor values
def get_sensor_data():
    temperature = round(random.uniform(20, 40), 2)
    humidity = round(random.uniform(30, 90), 2)
    light_intensity = random.randint(100, 1000)
    return temperature, humidity, light_intensity

# Display and update real-time sensor values
if "sensor_data" not in st.session_state:
    st.session_state.sensor_data = get_sensor_data()

temperature, humidity, light_intensity = st.session_state.sensor_data
sensor_placeholder.markdown(f"""
    * *Temperature:* {temperature} °C  
    * *Humidity:* {humidity} %  
    * *Light Intensity:* {light_intensity} Lux
""")

# Update sensor data every second
if st.button("Refresh Sensor Data"):
    st.session_state.sensor_data = get_sensor_data()

# Prediction Section
st.subheader("Cocoon Yield Prediction")
if st.button("Predict Cocoon Yield"):
    # Fetch the latest sensor data
    temperature, humidity, light_intensity = st.session_state.sensor_data
    
    # Prepare input for prediction
    input_data = np.array([[temperature, humidity, light_intensity, feeding_quality]])
    
    # Predict cocoon yield
    cocoon_yield = model.predict(input_data)[0]
    st.success(f"Predicted Cocoon Yield: {cocoon_yield:.2f} kg")