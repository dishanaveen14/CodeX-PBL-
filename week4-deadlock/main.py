"""main.py - starts the Deadlock Simulation System. Owner: Team Lead.

Run with:  python main.py   (or  py main.py  on Windows)
"""
import tkinter as tk

from gui_main import MainApp


def main():
    root = tk.Tk()
    MainApp(root)
    root.mainloop()


if __name__ == '__main__':
    main()
