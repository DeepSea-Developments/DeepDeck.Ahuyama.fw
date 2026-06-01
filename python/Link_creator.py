import os
import ctypes
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import win32com.client

# --- CONFIGURATION ---
MACROS_FOLDER = r"C:\macros"

def prepare_hidden_folder(path):
    """Creates the folder if it doesn't exist and applies the 'Hidden' attribute"""
    if not os.path.exists(path):
        try:
            os.makedirs(path)
            FILE_ATTRIBUTE_HIDDEN = 0x02
            ctypes.windll.kernel32.SetFileAttributesW(path, FILE_ATTRIBUTE_HIDDEN)
        except Exception as e:
            messagebox.showerror("Error", f"Could not create the hidden folder:\n{e}")

def select_program():
    """Opens the file explorer to look for the .exe"""
    file_path = filedialog.askopenfilename(
        title="Select the program",
        filetypes=(("Executable Files", "*.exe"), ("All files", "*.*"))
    )
    if file_path:
        path_entry.delete(0, tk.END)
        path_entry.insert(0, file_path)
        
        # Suggest a shortcut name based on the selected file
        base_name = os.path.splitext(os.path.basename(file_path))[0].lower()
        alias_entry.delete(0, tk.END)
        alias_entry.insert(0, base_name)

def create_shortcut():
    """Takes the path and alias, generates the .lnk file, and updates the list"""
    target_path = path_entry.get()
    alias = alias_entry.get()

    if not target_path or not alias:
        messagebox.showwarning("Attention", "Please select a program and type an alias.")
        return

    # Clean up the alias (replace spaces with underscores)
    alias = alias.replace(" ", "_").lower()
    shortcut_path = os.path.join(MACROS_FOLDER, f"{alias}.lnk")

    try:
        shell = win32com.client.Dispatch("WScript.Shell")
        shortcut = shell.CreateShortCut(shortcut_path)
        shortcut.Targetpath = target_path
        shortcut.WorkingDirectory = os.path.dirname(target_path)
        shortcut.save()
        
        messagebox.showinfo("Success!", f"Shortcut created successfully:\n{alias}")
        
        # Clear fields and refresh the table
        path_entry.delete(0, tk.END)
        alias_entry.delete(0, tk.END)
        update_list()
        
    except Exception as e:
        messagebox.showerror("Error", f"There was a problem creating the shortcut:\n{e}")

def update_list():
    """Reads the hidden folder and updates the table with existing shortcuts"""
    # 1. Clear the current table
    for item in shortcut_table.get_children():
        shortcut_table.delete(item)
        
    if not os.path.exists(MACROS_FOLDER):
        return

    try:
        shell = win32com.client.Dispatch("WScript.Shell")
        # 2. Iterate through all files in the folder
        for file in os.listdir(MACROS_FOLDER):
            if file.endswith(".lnk"):
                full_path = os.path.join(MACROS_FOLDER, file)
                alias = file.replace(".lnk", "")
                
                # Read where the shortcut points to
                try:
                    shortcut = shell.CreateShortCut(full_path)
                    target = shortcut.Targetpath
                except:
                    target = "Unknown / Error reading"
                
                # 3. Insert into the table
                shortcut_table.insert("", tk.END, values=(alias, target))
    except Exception as e:
        print(f"Error reading shortcuts: {e}")

def open_folder():
    """Opens the hidden folder in Windows File Explorer"""
    prepare_hidden_folder(MACROS_FOLDER)
    os.startfile(MACROS_FOLDER)

def delete_selected_shortcut():
    """Deletes the selected shortcut from the table"""
    selection = shortcut_table.selection()
    if not selection:
        messagebox.showwarning("Attention", "Select a shortcut from the list to delete.")
        return
        
    item = shortcut_table.item(selection)
    alias = item['values'][0]
    shortcut_path = os.path.join(MACROS_FOLDER, f"{alias}.lnk")
    
    response = messagebox.askyesno("Confirm", f"Are you sure you want to delete the alias '{alias}'?")
    if response:
        try:
            os.remove(shortcut_path)
            update_list()
        except Exception as e:
            messagebox.showerror("Error", f"Could not delete the shortcut:\n{e}")

# --- GRAPHICAL USER INTERFACE (GUI) ---
prepare_hidden_folder(MACROS_FOLDER)

window = tk.Tk()
window.title("DeepDeck - Shortcut Manager")
window.geometry("550x550")
window.resizable(False, False)
window.config(padx=20, pady=20)

# Title
tk.Label(window, text="Local Apps Manager", font=("Arial", 14, "bold")).pack(pady=(0, 15))

# --- TOP SECTION: CREATION ---
top_frame = tk.Frame(window)
top_frame.pack(fill="x")

# File Path
path_frame = tk.Frame(top_frame)
path_frame.pack(fill="x", pady=5)
tk.Label(path_frame, text="1. Program Path:").pack(anchor="w")
path_entry = tk.Entry(path_frame, width=55)
path_entry.pack(side="left", padx=(0, 10))
btn_browse = tk.Button(path_frame, text="Browse...", command=select_program)
btn_browse.pack(side="left")

# Alias
alias_frame = tk.Frame(top_frame)
alias_frame.pack(fill="x", pady=10)
tk.Label(alias_frame, text="2. Alias (e.g., app1, spotify, ps):").pack(anchor="w")
alias_entry = tk.Entry(alias_frame, width=30)
alias_entry.pack(side="left", padx=(0, 20))

# Main Create Button
btn_create = tk.Button(alias_frame, text="Create Shortcut", bg="#4CAF50", fg="white", font=("Arial", 10, "bold"), command=create_shortcut)
btn_create.pack(side="left", ipadx=10)

tk.Frame(window, height=2, bg="gray").pack(fill="x", pady=15) # Divider line

# --- BOTTOM SECTION: LIST ---
tk.Label(window, text="Existing Shortcuts:", font=("Arial", 10, "bold")).pack(anchor="w", pady=(0, 5))

list_frame = tk.Frame(window)
list_frame.pack(fill="both", expand=True)

# Create the table (Treeview)
columns = ("Alias", "Target")
shortcut_table = ttk.Treeview(list_frame, columns=columns, show="headings", height=8)
shortcut_table.heading("Alias", text="Alias (It will type this)")
shortcut_table.column("Alias", width=120, anchor="w")
shortcut_table.heading("Target", text="Program Path")
shortcut_table.column("Target", width=360, anchor="w")
shortcut_table.pack(side="left", fill="both", expand=True)

# Scrollbar for the table
scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=shortcut_table.yview)
shortcut_table.configure(yscroll=scrollbar.set)
scrollbar.pack(side="right", fill="y")

# List management buttons
list_btn_frame = tk.Frame(window)
list_btn_frame.pack(fill="x", pady=10)

btn_delete = tk.Button(list_btn_frame, text="Delete Selected", fg="red", command=delete_selected_shortcut)
btn_delete.pack(side="left")

btn_open = tk.Button(list_btn_frame, text="View hidden folder", fg="blue", relief="flat", cursor="hand2", command=open_folder)
btn_open.pack(side="right")

# Populate the list on startup
update_list()

window.mainloop()