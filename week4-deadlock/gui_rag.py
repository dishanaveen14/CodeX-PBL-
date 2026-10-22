"""gui_rag.py - STUB. Owner: Student 3 (replace this whole file with the real code)."""
from tkinter import ttk


class RAGTab(ttk.Frame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        ttk.Label(self, text='Student 3 is working on this tab (Resource Allocation Graph)',
                  font=('Segoe UI', 14)).pack(expand=True)

    def refresh(self):
        """Redraw everything from self.app.state. Called after every change."""
        pass
