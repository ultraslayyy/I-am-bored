using System.Drawing.Printing;
using System.Security.Cryptography;
using System.Text;
using ScintillaNET;
using System.Media;
using System.Xml.Linq;

namespace test;

public partial class MainForm : Form
{
    private bool isFullScreen = false;
    private bool isPosted = false;
    private FormBorderStyle previousBorderStyle;
    private FormWindowState previousWindowState;
    private bool previousTopMost;

    private Encoding currentEncoding = new UTF8Encoding(false);
    private MenuStrip menu = null!;
    private TabControl tabControl = null!;

    public MainForm()
    {
        Height = 700;
        Width = 1024;

        SuspendLayout();
        InitializeTabs();
        InitializeMenu();
        AddNewTab();
        AddNewTab();
        ResumeLayout(true);
    }

    #region Tabs & Editor

    private void InitializeTabs()
    {
        tabControl = new TabControl
        {
            Dock = DockStyle.Fill
        };

        Controls.Add(tabControl);
    }

    private ToolStripMenuItem deleteItem = null!;

    private void AddNewTab(string title = "Untitled", string text = "")
    {
        var page = new TabPage(title);
        var editor = new Scintilla
        {
            Dock = DockStyle.Fill,
            Font = new Font("Consolas", 12),
            Text = text
        };

        editor.Margins[0].Width = 40;

        editor.LexerName = "cpp";
        editor.Styles[Style.Cpp.Default].ForeColor = Color.Black;
        editor.Styles[Style.Cpp.Comment].ForeColor = Color.Green;
        editor.Styles[Style.Cpp.Number].ForeColor = Color.DarkOrange;
        editor.Styles[Style.Cpp.String].ForeColor = Color.Brown;
        editor.Styles[Style.Cpp.Word].ForeColor = Color.Blue;

        editor.SetKeywords(0, string.Join(" ", csharpKeywords));

        editor.UpdateUI += Editor_UpdateUI;

        page.Controls.Add(editor);
        tabControl.TabPages.Add(page);
        tabControl.SelectedTab = page;

        AttachAutocomplete(editor);
    }

    private Scintilla? CurrentEditor => tabControl.SelectedTab?.Controls[0] as Scintilla;

    private void AttachAutocomplete(Scintilla editor)
    {
        editor.CharAdded += Editor_CharAdded;
    }

    private void Editor_UpdateUI(object? sender, UpdateUIEventArgs e)
    {
        if (sender is Scintilla editor && editor == CurrentEditor)
        {
            deleteItem.Enabled = editor.SelectionEnd != editor.SelectionStart;
        }
    }

    private void Editor_CharAdded(object? sender, CharAddedEventArgs e)
    {
        if (sender is not Scintilla scintilla) return;

        int currentPos = scintilla.CurrentPosition;
        int wordStartPost = scintilla.WordStartPosition(currentPos, true);
        int lenEntered = currentPos - wordStartPost;
        if (File.Exists(@"autoCompletion\cs.xml"))
        {
            XDocument doc = XDocument.Load("autoCompletion/cs.xml");
            var keywords = doc.Descendants("KeyWord")
                .Select(k => new
                {
                    Name = (string?)k.Attribute("name"),
                    isFunction = ((string?)k.Attribute("Func")) == "yes",
                    Overloads = k.Elements("Overload")
                        .Select(o => new
                        {
                            ReturnType = (string?)o.Attribute("retVal"),
                            Params = o.Elements("Param").Select(p => (string?)p.Attribute("name")).ToArray()
                        }).ToArray()
                })
                .ToArray();

            if (lenEntered > 0 && !scintilla.AutoCActive)
            {
                string currentWord = scintilla.GetTextRange(wordStartPost, lenEntered);

                var matches = keywords
                    .Where(k => k.Name!.StartsWith(currentWord, StringComparison.OrdinalIgnoreCase))
                    .Select(k =>
                    {
                        if (!k.isFunction)
                            return k.Name;

                        return string.Join(" ", k.Overloads.Select(o =>
                            $"{k.Name}({string.Join(", ", o.Params)})"));
                    })
                    .ToArray();

                if (matches.Length > 0)
                {
                    scintilla.AutoCShow(lenEntered, string.Join(" ", matches));
                }
            }
        }
    }

    #endregion

    #region Menu

    private ToolStripMenuItem fileMenu = null!;
    private ToolStripMenuItem editMenu = null!;
    private ToolStripMenuItem searchMenu = null!;
    private ToolStripMenuItem viewMenu = null!;
    private ToolStripMenuItem encodingMenu = null!;
    private ToolStripMenuItem toolsMenu = null!;
    private ToolStripMenuItem helpMenu = null!;

    private void InitializeMenu()
    {
        menu = new()
        {
            Dock = DockStyle.Top
        };
        MainMenuStrip = menu;
        Controls.Add(menu);

        ToolStripManager.RenderMode = ToolStripManagerRenderMode.System;

        fileMenu = new ToolStripMenuItem("File");
        var newItem = new ToolStripMenuItem("New", null, (s, e) => AddNewTab())
        {
            ShortcutKeys = Keys.Control | Keys.N
        };
        var openItem = new ToolStripMenuItem("Open", null, OpenFile)
        {
            ShortcutKeys = Keys.Control | Keys.O
        };
        var saveItem = new ToolStripMenuItem("Save", null, SaveFile)
        {
            ShortcutKeys = Keys.Control | Keys.S
        };
        var saveAsItem = new ToolStripMenuItem("Save As", null, SaveFileAs)
        {
            ShortcutKeys = Keys.Control | Keys.Alt | Keys.S
        };
        var printItem = new ToolStripMenuItem("Print", null, PrintItem_Click)
        {
            ShortcutKeys = Keys.Control | Keys.P
        };
        var exitItem = new ToolStripMenuItem("Exit", null, (s, e) => Close())
        {
            ShortcutKeys = Keys.Alt | Keys.F4
        };

        fileMenu.DropDownItems.AddRange([
            newItem,
            openItem,
            saveItem,
            saveAsItem,
            new ToolStripSeparator(),
            printItem,
            new ToolStripSeparator(),
            exitItem
        ]);

        editMenu = new ToolStripMenuItem("Edit");
        var undoItem = new ToolStripMenuItem("Undo", null, (s, e) => CurrentEditor?.Undo())
        {
            ShortcutKeys = Keys.Control | Keys.Z
        };
        var redoItem = new ToolStripMenuItem("Redo", null, (s, e) => CurrentEditor?.Redo())
        {
            ShortcutKeys = Keys.Control | Keys.Y
        };
        var cutItem = new ToolStripMenuItem("Cut", null, (s, e) => CurrentEditor?.Cut())
        {
            ShortcutKeys = Keys.Control | Keys.X
        };
        var copyItem = new ToolStripMenuItem("Copy", null, (s, e) => CurrentEditor?.Copy())
        {
            ShortcutKeys = Keys.Control | Keys.C
        };
        var pasteItem = new ToolStripMenuItem("Paste", null, (s, e) => CurrentEditor?.Paste())
        {
            ShortcutKeys = Keys.Control | Keys.V
        };
        deleteItem = new ToolStripMenuItem("Delete", null, (s, e) => CurrentEditor?.ReplaceSelection(""))
        {
            ShortcutKeys = Keys.Delete,
            Enabled = false
        };
        var selectAllItem = new ToolStripMenuItem("Select All", null, (s, e) => CurrentEditor?.SelectAll())
        {
            ShortcutKeys = Keys.Control | Keys.A
        };
        var beginEndSelectItem = new ToolStripMenuItem("Begin/End Select", null, ToggleSelection)
        {
            ShortcutKeys = Keys.Control | Keys.Shift | Keys.B,
            CheckOnClick = true,
            Checked = false
        };
        var insertDropdown = new ToolStripMenuItem("Insert", null);
        var insertDateTimeShortItem = new ToolStripMenuItem("Date Time (short)", null, (s, e) => InsertDate(DateTypeEnum.Short));
        var insertDateTimeLongItem = new ToolStripMenuItem("Date Time (long)", null, (s, e) => InsertDate(DateTypeEnum.Long));

        insertDropdown.DropDownItems.AddRange([
            insertDateTimeShortItem,
            insertDateTimeLongItem
        ]);

        var convertCaseToDropdown = new ToolStripMenuItem("Convert Case to", null);
        var convertCaseToUppercaseItem = new ToolStripMenuItem("UPPERCASE", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.UPPERCASE))
        {
            ShortcutKeys = Keys.Control | Keys.Shift | Keys.U
        };
        var convertCaseToLowercaseItem = new ToolStripMenuItem("lowercase", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.lowercase))
        {
            ShortcutKeys = Keys.Control | Keys.U
        };
        var convertCaseToPropercaseItem = new ToolStripMenuItem("Proper Case", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.ProperCase))
        {
            ShortcutKeys = Keys.Alt | Keys.U
        };
        var convertCaseToPropercaseBlendItem = new ToolStripMenuItem("Proper Case (blend)", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.ProperCaseblend))
        {
            ShortcutKeys = Keys.Alt | Keys.U
        };
        var convertCaseToSentencecaseItem = new ToolStripMenuItem("Sentence case", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.SentenceCase))
        {
            ShortcutKeys = Keys.Control | Keys.Alt | Keys.U
        };
        var convertCaseToSentencecaseBlendItem = new ToolStripMenuItem("Sentence case (blend)", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.SetenceCaseblend))
        {
            ShortcutKeys = Keys.Control | Keys.Alt | Keys.Shift | Keys.U
        };
        var convertCaseToInvertcaseItem = new ToolStripMenuItem("iNVERT cASE", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.iNVERTcASE));
        var convertCaseToRandomcaseItem = new ToolStripMenuItem("ranDOm CasE", null, (s, e) => ConvertSelectionCase(SelectionCaseEnum.ranDOmCasE));

        convertCaseToDropdown.DropDownItems.AddRange([
            convertCaseToUppercaseItem,
            convertCaseToLowercaseItem,
            convertCaseToPropercaseItem,
            convertCaseToPropercaseBlendItem,
            convertCaseToSentencecaseItem,
            convertCaseToSentencecaseBlendItem,
            convertCaseToInvertcaseItem,
            convertCaseToRandomcaseItem
        ]);

        editMenu.DropDownItems.AddRange([
            undoItem,
            redoItem,
            new ToolStripSeparator(),
            cutItem,
            copyItem,
            pasteItem,
            deleteItem,
            selectAllItem,
            beginEndSelectItem,
            new ToolStripSeparator(),
            insertDropdown,
            convertCaseToDropdown
        ]);

        searchMenu = new ToolStripMenuItem("Search");
        var searchFindItem = new ToolStripMenuItem("Find...", null, (s, e) =>
        {
            if (CurrentEditor == null) return;
            var dlg = new FindReplaceDialog(CurrentEditor)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            dlg.Show(this);
        })
        {
            ShortcutKeys = Keys.Control | Keys.F
        };
        var searchGotoItem = new ToolStripMenuItem("Go to...", null, (s, e) =>
        {
            if (CurrentEditor == null) return;
            var dlg = new GotoDialog(CurrentEditor)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            dlg.Show(this);
        })
        {
            ShortcutKeys = Keys.Control | Keys.G
        };

        searchMenu.DropDownItems.AddRange([
            searchFindItem,
            searchGotoItem
        ]);

        viewMenu = new ToolStripMenuItem("View");
        var alwaysOnTopItem = new ToolStripMenuItem("Always on Top")
        {
            CheckOnClick = true,
            Checked = false
        };
        alwaysOnTopItem.CheckedChanged += (s, e) =>
        {
            TopMost = alwaysOnTopItem.Checked;
        };
        var toggleFullScreenModeItem = new ToolStripMenuItem("Toggle Full Screen Mode", null, (s, e) => ToggleFullScreen())
        {
            ShortcutKeys = Keys.F11
        };
        var postItItem = new ToolStripMenuItem("Post-It", null, (s, e) => TogglePostIt())
        {
            ShortcutKeys = Keys.F12
        };

        viewMenu.DropDownItems.AddRange([
            alwaysOnTopItem,
            toggleFullScreenModeItem,
            postItItem
        ]);

        encodingMenu = new ToolStripMenuItem("Encoding");
        var encodingAnsiItem = new ToolStripMenuItem("ANSI", null);
        encodingAnsiItem.Click += (s, e) => SetEncoding(Encoding.Default, encodingAnsiItem);
        var encodingUtf8Item = new ToolStripMenuItem("UTF-8", null)
        {
            Checked = true
        };
        encodingUtf8Item.Click += (s, e) => SetEncoding(new UTF8Encoding(false), encodingUtf8Item);
        var encodingUtf8BomItem = new ToolStripMenuItem("UTF-8-BOM", null);
        encodingUtf8BomItem.Click += (s, e) => SetEncoding(new UTF8Encoding(true), encodingUtf8BomItem);
        var encodingUtf8BeBomItem = new ToolStripMenuItem("UTF-8 BE BOM", null);
        encodingUtf8BeBomItem.Click += (s, e) => SetEncoding(new UnicodeEncoding(true, true), encodingUtf8BeBomItem);
        var encodingUtf8LeBomItem = new ToolStripMenuItem("UTF-8 LE BOM", null);
        encodingUtf8LeBomItem.Click += (s, e) => SetEncoding(new UnicodeEncoding(false, true), encodingUtf8LeBomItem);

        encodingMenu.DropDownItems.AddRange([
            encodingAnsiItem,
            encodingUtf8Item,
            encodingUtf8BomItem,
            encodingUtf8BeBomItem,
            encodingUtf8LeBomItem
        ]);

        toolsMenu = new ToolStripMenuItem("Tools");
        var md5Dropdown = new ToolStripMenuItem("MD5");
        var md5GenerateItem = new ToolStripMenuItem("Generate...", null, (s, e) =>
        {
            var popup = new ToolsGeneratePopup(HashAlgorithms.MD5)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            popup.Show(this);
        });
        md5Dropdown.DropDownItems.AddRange([md5GenerateItem]);

        var sha1Dropdown = new ToolStripMenuItem("SHA-1");
        var sha1GenerateItem = new ToolStripMenuItem("Generate...", null, (s, e) =>
        {
            var popup = new ToolsGeneratePopup(HashAlgorithms.SHA1)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            popup.Show(this);
        });
        sha1Dropdown.DropDownItems.AddRange([sha1GenerateItem]);

        var sha256Dropdown = new ToolStripMenuItem("SHA-256");
        var sha256GenerateItem = new ToolStripMenuItem("Generate...", null, (s, e) =>
        {
            var popup = new ToolsGeneratePopup(HashAlgorithms.SHA256)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            popup.Show(this);
        });
        sha256Dropdown.DropDownItems.AddRange([sha256GenerateItem]);

        var sha512Dropdown = new ToolStripMenuItem("SHA-512");
        var sha512GenerateItem = new ToolStripMenuItem("Generate...", null, (s, e) =>
        {
            var popup = new ToolsGeneratePopup(HashAlgorithms.SHA512)
            {
                ShowInTaskbar = false,
                StartPosition = FormStartPosition.CenterParent
            };
            popup.Show(this);
        });
        sha512Dropdown.DropDownItems.AddRange([sha512GenerateItem]);

        toolsMenu.DropDownItems.AddRange([
            md5Dropdown,
            sha1Dropdown,
            sha256Dropdown,
            sha512Dropdown
        ]);

        helpMenu = new ToolStripMenuItem("Help");
        helpMenu.DropDownItems.Add("About", null, (s, e) =>
            MessageBox.Show("Notepad#", "About"));

        menu.Items.AddRange([
            fileMenu,
            editMenu,
            searchMenu,
            viewMenu,
            encodingMenu,
            toolsMenu,
            helpMenu
        ]);
    }

    private void ToggleFullScreen()
    {
        if (!isFullScreen)
        {
            previousBorderStyle = FormBorderStyle;
            previousWindowState = WindowState;
            previousTopMost = TopMost;

            FormBorderStyle = FormBorderStyle.None;
            WindowState = FormWindowState.Maximized;
            TopMost = true;
            isFullScreen = true;
            MainMenuStrip?.Hide();
        }
        else
        {
            FormBorderStyle = previousBorderStyle;
            WindowState = previousWindowState;
            TopMost = previousTopMost;
            isFullScreen = false;
            MainMenuStrip?.Show();
        }
    }

    private void TogglePostIt()
    {
        if (!isPosted)
        {
            previousBorderStyle = FormBorderStyle;
            previousTopMost = TopMost;

            FormBorderStyle = FormBorderStyle.None;
            TopMost = true;
            isPosted = true;
            MainMenuStrip?.Hide();
        }
        else
        {
            FormBorderStyle = previousBorderStyle;
            TopMost = previousTopMost;
            isPosted = false;
            MainMenuStrip?.Show();
        }
    }

    private int? selectionAnchor = null;

    private void ToggleSelection(object? sender, EventArgs e)
    {
        var editor = CurrentEditor;
        if (editor == null) return;

        if (selectionAnchor == null)
        {
            selectionAnchor = editor.CurrentPosition;
        }
        else
        {
            int start = Math.Min(selectionAnchor.Value, editor.CurrentPosition);
            int end = Math.Max(selectionAnchor.Value, editor.CurrentPosition);

            editor.SelectionStart = start;
            editor.SelectionEnd = end;

            selectionAnchor = null;
        }
    }

    private enum DateTypeEnum
    {
        Short,
        Long
    }

    private void InsertDate(DateTypeEnum type)
    {
        var editor = CurrentEditor;
        if (editor == null) return;

        var timeAndDate = DateTime.Now;
        if (type == DateTypeEnum.Short)
        {
            editor.InsertText(-1, $"{timeAndDate:t} {timeAndDate:d}");
        }
        else
        {
            editor.InsertText(-1, $"{timeAndDate:T} {timeAndDate:D}");
        }
    }

    private enum SelectionCaseEnum
    {
        UPPERCASE,
        lowercase,
        ProperCase,
        ProperCaseblend,
        SentenceCase,
        SetenceCaseblend,
        iNVERTcASE,
        ranDOmCasE
    }

    private void ConvertSelectionCase(SelectionCaseEnum selectedCase)
    {
        var editor = CurrentEditor;
        if (editor == null) return;

        int start = editor.SelectionStart;
        int end = editor.SelectionEnd;

        if (end == start) return;

        string selectedText = editor.SelectedText;
        string convertedText = selectedCase switch
        {
            SelectionCaseEnum.UPPERCASE => selectedText.ToUpper(),
            SelectionCaseEnum.lowercase => selectedText.ToLower(),
            SelectionCaseEnum.ProperCase => System.Globalization.CultureInfo.CurrentCulture.TextInfo.ToTitleCase(selectedText.ToLower()),
            SelectionCaseEnum.ProperCaseblend => string.Join(" ", selectedText.Split(' ').Select(word => word.Length > 0 ? char.ToUpper(word[0]) + word[1..] : word)),
            SelectionCaseEnum.SentenceCase => ToSentenceCase(selectedText),
            SelectionCaseEnum.SetenceCaseblend => ToSentenceCaseBlend(selectedText),
            SelectionCaseEnum.iNVERTcASE => InvertCase(selectedText),
            SelectionCaseEnum.ranDOmCasE => RandomCase(selectedText),
            _ => selectedText
        };

        editor.ReplaceSelection(convertedText);
        editor.SelectionStart = start;
        editor.SelectionEnd = end;
    }

    private static string ToSentenceCase(string text)
    {
        char[] result = text.ToLower().ToCharArray();
        bool newSentence = true;

        for (int i = 0; i < result.Length; i++)
        {
            if (char.IsLetter(result[i]) && newSentence)
            {
                result[i] = char.ToUpper(result[i]);
                newSentence = false;
            }

            if (result[i] == '.' || result[i] == '!' || result[i] == '?')
            {
                newSentence = true;
            }
        }

        return new string(result);
    }

    private static string ToSentenceCaseBlend(string text)
    {
        if (string.IsNullOrEmpty(text)) return text;
        return char.ToUpper(text[0]) + text[1..];
    }

    private static string InvertCase(string text)
    {
        return new string([.. text.Select(c => char.IsLetter(c) ? (char.IsUpper(c) ? char.ToLower(c) : char.ToUpper(c)) : c)]);
    }

    private static string RandomCase(string text)
    {
        Random rnd = new();
        return new string([.. text.Select(c => char.IsLetter(c) ? (rnd.Next(2) == 0 ? char.ToLower(c) : char.ToUpper(c)) : c)]);
    }

    private void SetEncoding(Encoding encoding, ToolStripMenuItem clickedItem)
    {
        currentEncoding = encoding;
        foreach (ToolStripMenuItem item in encodingMenu.DropDownItems)
        {
            item.Checked = false;
        }
        clickedItem.Checked = true;
    }

    #endregion

    #region Printing

    public void PrintItem_Click(object? sender, EventArgs? e)
    {
        var editor = CurrentEditor;
        if (editor == null) return;

        using PrintDocument printDoc = new();
        string textToPrint = editor.SelectedText.Length > 0 ? editor.SelectedText : editor.Text;
        string[] lines = textToPrint.Split(["\r\n", "\n"], StringSplitOptions.None);
        int startPos = 0;

        printDoc.PrintPage += (s, ev) =>
        {
            int linesPrinted = 0;
            int lineHeight = editor.Lines[0].Height;
            int linesPerPage = ev.MarginBounds.Height / lineHeight;

            for (int i = startPos; i < lines.Length && linesPrinted < linesPerPage; i++)
            {
                ev.Graphics?.DrawString(
                    lines[i],
                    editor.Font,
                    Brushes.Black,
                    ev.MarginBounds.Left,
                    ev.MarginBounds.Top + linesPrinted * lineHeight
                );
                linesPrinted++;
                startPos = i + 1;
            }

            ev.HasMorePages = startPos < lines.Length;
        };

        using PrintDialog dlg = new()
        {
            Document = printDoc,
            UseEXDialog = true
        };

        if (dlg.ShowDialog() == DialogResult.OK)
        {
            startPos = 0;
            printDoc.PrinterSettings = dlg.PrinterSettings;
            printDoc.Print();
        }
    }

    #endregion

    #region File Handlers

    private void OpenFile(object? sender, EventArgs? e)
    {
        using OpenFileDialog ofd = new()
        {
            Filter = "All Files (*.*)|*.*"
        };

        if (ofd.ShowDialog() == DialogResult.OK)
        {
            string text = File.ReadAllText(ofd.FileName, currentEncoding);
            AddNewTab(Path.GetFileName(ofd.FileName), text);
        }
    }

    private void SaveFile(object? sender, EventArgs? e)
    {
        if (CurrentEditor == null) return;

        using SaveFileDialog sfd = new()
        {
            Filter = "All Files (*.*)|*.*"
        };

        if (sfd.ShowDialog() == DialogResult.OK)
        {
            File.WriteAllText(sfd.FileName, CurrentEditor.Text, currentEncoding);
            if (tabControl.SelectedTab != null)
                tabControl.SelectedTab.Text = Path.GetFileName(sfd.FileName);
        }
    }

    private void SaveFileAs(object? sender, EventArgs? e)
    {
        SaveFile(sender, e);
    }

    #endregion

    #region Keywords

    private readonly string[] csharpKeywords =
    [
        "abstract","as","base","bool","break","byte","case","catch","char","checked",
        "class","const","continue","decimal","default","delegate","do","double","else",
        "enum","event","explicit","extern","false","finally","fixed","float","for","foreach",
        "goto","if","implicit","in","int","interface","internal","is","lock","long","namespace",
        "new","null","object","operator","out","override","params","partial","private","protected","public",
        "readonly","ref","return","sbyte","sealed","short","sizeof","stackalloc","static",
        "string","struct","switch","this","throw","true","try","typeof","uint","ulong",
        "unchecked","unsafe","ushort","using","virtual","void","volatile","while"
    ];

    #endregion
}

#region FindReplaceDialog

public partial class FindReplaceDialog : Form
{
    private Scintilla editor = null!;

    private TabControl tabControl = null!;
    private TabPage tabFind = null!;
    private TabPage tabReplace = null!;
    private TabPage tabFindInFiles = null!;
    private TabPage tabFindInProjects = null!;
    private TabPage tabMark = null!;

    private ComboBox cmbFindWhat = null!;
    private CheckBox chkMatchCase = null!;
    private Label lblStatus = null!;

    bool dragging = false;
    int dragStartX = 0;
    int originalWidth;
    int originalFindWhatWidth;

    private bool transparencyEnabled = true;
    private bool transparencyOnFocusLost = true;
    private int transparencyOpacity = 60;

    public FindReplaceDialog(Scintilla currentEditor)
    {
        editor = currentEditor;
        InitializeComponent();
    }

    private void InitializeComponent()
    {
        Text = "Find";
        Width = 640;
        Height = 360;
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        StartPosition = FormStartPosition.CenterParent;

        tabControl = new TabControl { Dock = DockStyle.Fill };

        tabFind = new TabPage("Find")
        {
            Font = new Font("MS Shell Dlg 2", 8F, FontStyle.Regular, GraphicsUnit.Point),
            BackColor = Color.FromArgb(249, 249, 249)
        };
        var lblFindWhat = new Label { Text = "&Find what:", Location = new Point(54, 12), AutoSize = true, UseMnemonic = true, UseCompatibleTextRendering = true };
        cmbFindWhat = new ComboBox { Location = new Point(110, 9), Width = 264, DropDownStyle = ComboBoxStyle.DropDown };

        var btnFindNext = new Button { Text = "Find Next", Top = 9, Width = 137, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        btnFindNext.Click += BtnFindNext_Click;
        var chkFindNext = new CheckBox { Top = 14, AutoSize = true, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        var gbInSelection = new GroupBox { Top = 29, Width = 315, Height = 76, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        var btnCount = new Button { Text = "Coun&t", Location = new Point(175, 9), Width = 137, UseMnemonic = true };
        btnCount.Click += BtnCount_Click;
        var chkInSelection = new CheckBox { Text = "In select&ion", Location = new Point(10, 45), Enabled = false, AutoSize = true, UseMnemonic = true };
        var btnFindAllCurrent = new Button { Text = "Find All in Current &Document", Location = new Point(175, 38), Width = 137, UseMnemonic = true, Height = 35 };

        gbInSelection.Controls.AddRange([btnCount, chkInSelection, btnFindAllCurrent]);

        var btnFindAllOpened = new Button { Text = "Find All in All &Opened Documents", Top = 108, Width = 137, Height = 35, UseMnemonic = true, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        var btnClose = new Button { Text = "Close", Top = 148, Width = 137, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        btnClose.Click += (s, e) => Close();

        var chkBackward = new CheckBox { Text = "Backward direction", Location = new Point(15, 105), AutoSize = true };
        var chkMatchWholeWord = new CheckBox { Text = "Match &whole word only", Location = new Point(15, 124), AutoSize = true, UseMnemonic = true };
        chkMatchCase = new CheckBox { Text = "Match &case", Location = new Point(15, 143), AutoSize = true, Checked = true, UseMnemonic = true };
        var chkWrapAround = new CheckBox { Text = "Wra&p around", Location = new Point(15, 162), AutoSize = true, Checked = true, UseMnemonic = true };

        var gbSearchMode = new GroupBox { Text = "Search Mode", Location = new Point(5, 189), Width = 300, Height = 80 };
        var rbNormal = new RadioButton { Text = "&Normal", Location = new Point(10, 19), AutoSize = true, Checked = true, UseMnemonic = true };
        var rbExtended = new RadioButton { Text = "E&xtended (\\n, \\r, \\t, \\0, ...)", Location = new Point(10, 39), AutoSize = true, UseMnemonic = true };
        var rbRegex = new RadioButton { Text = "Re&gular expression", Location = new Point(10, 58), AutoSize = true, UseMnemonic = true };
        var chkMatchNewline = new CheckBox { Text = "&. matches newline", Location = new Point(136, 60), AutoSize = true, Enabled = false, UseMnemonic = true };

        gbSearchMode.Controls.AddRange([rbNormal, rbExtended, rbRegex, chkMatchNewline]);

        var gbTransparency = new GroupBox { Top = 189, Width = 151, Height = 79, Anchor = AnchorStyles.Top | AnchorStyles.Right };
        var chkTransparency = new CheckBox
        {
            Text = "Transparenc&y",
            Top = 189,
            AutoSize = true,
            UseMnemonic = true,
            Checked = true,
            Anchor = AnchorStyles.Top | AnchorStyles.Right
        };
        var rbOnLosingFocus = new RadioButton { Text = "On losing focus", Location = new Point(15, 18), AutoSize = true, Checked = true };
        var rbAlways = new RadioButton { Text = "Always", Location = new Point(15, 38), AutoSize = true };
        var trackTransparency = new TrackBar { Location = new Point(8, 55), Width = 133, Minimum = 0, Maximum = 100, Value = 60, TickStyle = TickStyle.None, Enabled = false, Height = 24 };
        trackTransparency.ValueChanged += (s, e) =>
        {
            transparencyOpacity = trackTransparency.Value;
            ApplyTransparency();
        };
        rbOnLosingFocus.CheckedChanged += (s, e) =>
        {
            if (rbOnLosingFocus.Checked)
            {
                transparencyOnFocusLost = true;
                trackTransparency.Enabled = false;
            }
            else
            {
                transparencyOnFocusLost = false;
                trackTransparency.Enabled = true;
            }
            ApplyTransparency();
        };
        chkTransparency.CheckedChanged += (s, e) =>
        {
            transparencyEnabled = chkTransparency.Checked;
            rbOnLosingFocus.Enabled = chkTransparency.Checked;
            rbAlways.Enabled = chkTransparency.Checked;
            trackTransparency.Enabled = chkTransparency.Checked;
            ApplyTransparency();
        };

        gbTransparency.Controls.AddRange([rbOnLosingFocus, rbAlways, trackTransparency]);

        tabFind.Controls.AddRange([
            lblFindWhat, cmbFindWhat, btnFindNext, chkFindNext,
            gbInSelection, btnFindAllOpened, btnClose,
            chkBackward, chkMatchWholeWord, chkMatchCase, chkWrapAround,
            gbSearchMode, chkTransparency, gbTransparency
        ]);

        tabReplace = new TabPage("Replace");
        tabFindInFiles = new TabPage("Find in Files");
        tabFindInProjects = new TabPage("Find in Projects");
        tabMark = new TabPage("Mark");

        tabControl.TabPages.AddRange([tabFind, tabReplace, tabFindInFiles, tabFindInProjects, tabMark]);

        var bottomBar = new Panel
        {
            Dock = DockStyle.Bottom,
            Height = 22,
            BackColor = Color.FromArgb(240, 240, 240)
        };
        bottomBar.Paint += (s, e) =>
        {
            using var pen = new Pen(Color.FromArgb(215, 215, 215), 1);
            e.Graphics.DrawLine(pen, 0, 0, bottomBar.Width, 0);
        };
        lblStatus = new Label
        {
            AutoSize = true,
            ForeColor = Color.Blue,
            Location = new Point(0, 3)
        };
        var grip = new PictureBox
        {
            Dock = DockStyle.Right,
            Width = 16,
            Height = bottomBar.Height,
            BackColor = Color.Transparent,
            Cursor = Cursors.SizeWE
        };
        grip.Paint += (s, e) =>
        {
            using var brush = new SolidBrush(Color.LightGray);
            e.Graphics.FillPolygon(brush,
            [
                new Point(grip.Width - 1, grip.Height - 1),
                new Point(grip.Width - 9, grip.Height - 1),
                new Point(grip.Width - 1, grip.Height - 9)
            ]);
        };
        grip.MouseDown += (s, e) =>
        {
            if (e.Button == MouseButtons.Left)
            {
                dragging = true;
                dragStartX = Cursor.Position.X;
                originalWidth = Width;
                originalFindWhatWidth = cmbFindWhat.Width;
            }
        };
        grip.MouseUp += (s, e) => dragging = false;
        grip.MouseMove += (s, e) =>
        {
            if (!dragging) return;

            int delta = Cursor.Position.X - dragStartX;
            int newWidth = originalWidth + delta;
            int newFindWhatWidth = originalFindWhatWidth + delta;

            if (newWidth < 640) newWidth = 640;
            Width = newWidth;
            if (newFindWhatWidth < 264) newFindWhatWidth = 264;
            cmbFindWhat.Width = newFindWhatWidth;
        };

        bottomBar.Controls.AddRange([lblStatus, grip]);

        Controls.AddRange([tabControl, bottomBar]);
        Controls.SetChildIndex(bottomBar, 0);

        Load += (s, e) =>
        {
            btnFindNext.Left = ClientSize.Width - btnFindNext.Width - 36;
            chkFindNext.Left = ClientSize.Width - chkFindNext.Width - 15;
            gbInSelection.Left = ClientSize.Width - gbInSelection.Width - 36;
            btnFindAllOpened.Left = ClientSize.Width - btnFindAllOpened.Width - 36;
            btnClose.Left = ClientSize.Width - btnClose.Width - 36;
            gbTransparency.Left = ClientSize.Width - gbTransparency.Width - 36;
            chkTransparency.Left = ClientSize.Width - chkTransparency.Width - 100;
        };

        ApplyTransparency();
    }

    protected override void OnFormClosing(FormClosingEventArgs e)
    {
        base.OnFormClosing(e);

        Activated -= Form_Activated;
        Deactivate -= Form_Deactivate;

        Opacity = 1.0;
    }

    private void ApplyTransparency()
    {
        Activated -= Form_Activated;
        Deactivate -= Form_Deactivate;

        if (!transparencyEnabled)
        {
            Opacity = 1.0;
            return;
        }

        double opacityValue = Math.Clamp(transparencyOpacity / 100.0, 0.0, 1.0);

        if (transparencyOnFocusLost)
        {
            Activated += Form_Activated;
            Deactivate += Form_Deactivate;

            bool isActive = (ActiveForm == this) || ContainsFocus;
            Opacity = isActive ? 1.0 : opacityValue;
        }
        else
        {
            Opacity = opacityValue;
        }
    }

    private void Form_Activated(object? sender, EventArgs e)
    {
        if (IsDisposed || Disposing) return;
        Opacity = 1.0;
    }

    private void Form_Deactivate(object? sender, EventArgs e)
    {
        if (IsDisposed || Disposing) return;
        Opacity = Math.Clamp(transparencyOpacity / 100.0, 0.0, 1.0);
    }

    private void BtnFindNext_Click(object? sender, EventArgs e)
    {
        string searchText = cmbFindWhat.Text;
        if (string.IsNullOrEmpty(searchText)) return;

        int startPos = editor.SelectionEnd;
        StringComparison comparison = chkMatchCase.Checked ? StringComparison.Ordinal : StringComparison.OrdinalIgnoreCase;

        int foundPos = editor.Text.IndexOf(searchText, startPos, comparison);

        if (foundPos == -1 && startPos > 0)
        {
            foundPos = editor.Text.IndexOf(searchText, 0, comparison);
        }

        if (foundPos >= 0)
        {
            editor.SelectionStart = foundPos;
            editor.SelectionEnd = foundPos + searchText.Length;
            editor.ScrollCaret();
        }
        else
        {
            SystemSounds.Exclamation.Play();
            ShowMessage($"Find: Can't find the text \"{searchText}\" in entire file", true);
        }
    }

    private void BtnCount_Click(object? sender, EventArgs e)
    {
        string searchText = cmbFindWhat.Text;
        string sourceText = editor.Text;
        bool matchCase = chkMatchCase.Checked;

        if (string.IsNullOrWhiteSpace(searchText) || string.IsNullOrEmpty(sourceText))
        {
            ShowMessage($"Count: 0 matches in entire file");
            return;
        }

        StringComparison comparison = matchCase ? StringComparison.Ordinal : StringComparison.OrdinalIgnoreCase;
        int count = 0;
        int pos = 0;

        while ((pos = sourceText.IndexOf(searchText, pos, comparison)) >= 0)
        {
            count++;
            pos += searchText.Length;
        }

        ShowMessage($"Count: {count} match{(count > 1 ? "es" : "")} in entire file");
    }

    private void ShowMessage(string text, bool isError = false)
    {
        lblStatus.Text = text;
        lblStatus.ForeColor = isError ? Color.Red : Color.Blue;
    }
}

#endregion

#region GotoDialog

public partial class GotoDialog : Form
{
    private readonly Scintilla editor = null!;

    private NumericUpDown numGoTo = null!;
    private Label numNotFurther = null!;

    public GotoDialog(Scintilla currentEditor)
    {
        editor = currentEditor;
        InitializeComponent();
    }

    private void InitializeComponent()
    {
        Text = "Go To...";
        Width = 400;
        Height = 160;
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        StartPosition = FormStartPosition.CenterParent;
        Font = new Font("MS Shell Dlg 2", 8F, FontStyle.Regular, GraphicsUnit.Point);

        var rbLine = new RadioButton { Text = "&Line", Location = new Point(12, 6), Checked = true, AutoSize = true, UseMnemonic = true };
        var rbOffset = new RadioButton { Text = "&Offset", Location = new Point(148, 6), AutoSize = true, UseMnemonic = true };

        var lblHere = new Label { Text = "You are here:", Location = new Point(12, 41), AutoSize = true };
        var numHere = new NumericUpDown { Location = new Point(157, 41), Width = 106, Value = editor.CurrentLine + 1 };

        var lblGoTo = new Label { Text = "You want to &go to:", Location = new Point(12, 66), AutoSize = true, UseMnemonic = true };
        numGoTo = new NumericUpDown { Location = new Point(157, 66), Width = 106 };
        var btnGo = new Button { Text = "Go", Location = new Point(268, 66), Width = 106 };
        btnGo.Click += BtnGoTo_Click;

        var lblNotFurther = new Label { Text = "You can't go further than:", Location = new Point(12, 91), AutoSize = true };
        numNotFurther = new Label { Text = editor.Lines.Count.ToString(), Location = new Point(157, 91), AutoSize = true };
        var btnNowhere = new Button { Text = "I'm going nowhere", Location = new Point(268, 91), Width = 106 };
        btnNowhere.Click += (s, e) => Close();

        Controls.AddRange([
            rbLine, rbOffset,
            lblHere, numHere,
            lblGoTo, numGoTo, btnGo,
            lblNotFurther, numNotFurther, btnNowhere
        ]);
    }

    private void BtnGoTo_Click(object? sender, EventArgs e)
    {
        int line = (int)Math.Round(numGoTo.Value);
        int maxLine = int.Parse(numNotFurther.Text);
        if (line < 1) line = 1;
        if (line > maxLine) line = maxLine;

        int zeroBasedLine = line - 1;

        editor.Lines[zeroBasedLine].Goto();
        editor.ScrollCaret();
        editor.Focus();
    }
}

#endregion

#region Tools Popup

public enum HashAlgorithms
{
    MD5,
    SHA1,
    SHA256,
    SHA512
}

public class ToolsGeneratePopup : Form
{
    public CheckBox checkBox;
    public TextBox inputBox;
    public TextBox outputBox;
    public Button copyButton;
    public Button closeButton;
    public HashAlgorithms hashedType;

    public ToolsGeneratePopup(HashAlgorithms hashType)
    {
        hashedType = hashType;
        Text = $"Generate {hashType} Digest";
        Size = new Size(560, 360);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        StartPosition = FormStartPosition.CenterParent;
        MaximizeBox = false;
        MinimizeBox = false;
        TopMost = true;

        checkBox = new CheckBox { Text = "Treat each line as a separate string", Location = new Point(20, 10), AutoSize = true };
        Controls.Add(checkBox);

        inputBox = new TextBox
        {
            Location = new Point(20, 35),
            Width = 500,
            Height = 100,
            Multiline = true,
            ScrollBars = ScrollBars.Both
        };
        inputBox.TextChanged += InputBox_TextChanged;
        Controls.Add(inputBox);

        outputBox = new TextBox
        {
            Location = new Point(20, 145),
            Width = 500,
            Height = 100,
            Multiline = true,
            ReadOnly = true,
            ScrollBars = ScrollBars.Both,
            Text = "Output will appear here"
        };
        Controls.Add(outputBox);

        copyButton = new Button { Text = "Copy to Clipboard", Location = new Point(380, 250), Width = 140 };
        copyButton.Click += (s, e) =>
        {
            Clipboard.SetText(outputBox.Text);
            MessageBox.Show("Copied to clipboard!");
        };
        Controls.Add(copyButton);

        closeButton = new Button { Text = "Close", Location = new Point((ClientSize.Width - 90) / 2, 285), Width = 90 };
        closeButton.Click += (s, e) => Close();
        Controls.Add(closeButton);
    }

    private void InputBox_TextChanged(object? sender, EventArgs e)
    {
        if (string.IsNullOrWhiteSpace(outputBox.Text))
        {
            outputBox.Text = "";
        }

        if (hashedType == HashAlgorithms.MD5)
        {
            outputBox.Text = ComputeHash(inputBox.Text, MD5.Create());
        }
        else if (hashedType == HashAlgorithms.SHA1)
        {
            outputBox.Text = ComputeHash(inputBox.Text, SHA1.Create());
        }
        else if (hashedType == HashAlgorithms.SHA256)
        {
            outputBox.Text = ComputeHash(inputBox.Text, SHA256.Create());
        }
        else if (hashedType == HashAlgorithms.SHA512)
        {
            outputBox.Text = ComputeHash(inputBox.Text, SHA512.Create());
        }
    }

    private static string ComputeHash(string input, HashAlgorithm algorithm)
    {
        byte[] inputBytes = Encoding.UTF8.GetBytes(input);
        byte[] hashBytes = algorithm.ComputeHash(inputBytes);
        return Convert.ToHexStringLower(hashBytes);
    }
}

#endregion