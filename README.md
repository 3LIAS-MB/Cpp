sudo apt update && sudo apt install python3-pip -y
pip3 --version

sudo apt update && sudo apt install python3-venv -y
python3 -m venv venv
source venv/bin/activate
pip install pandas matplotlib seaborn numpy
