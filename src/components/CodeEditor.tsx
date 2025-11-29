import MonacoEditor from '@monaco-editor/react';
import { Terminal, Code, Cpu, FileCode } from 'lucide-react';

interface MachineCodeValue {
  assembly: string;
  binary: string;
  hex: string;
}

interface CodeEditorProps {
  title: string;
  subtitle: string;
  value: string | MachineCodeValue;
  onChange?: (value: string) => void;
  readOnly?: boolean;
  editable?: boolean;
  placeholder?: string;
  language?: string;
  theme?: 'light' | 'dark';
}

export function CodeEditor({
  title,
  subtitle,
  value,
  onChange,
  readOnly = false,
  editable = false,
  language = 'text',
  theme = 'dark',
}: CodeEditorProps) {
  const isDark = theme === 'dark';

  const isMachineCode = typeof value !== 'string' && title.includes('Machine');

  // === Helpers for styling ===
  const getIcon = () => {
    if (title.includes('Source')) return <FileCode className="w-4 h-4" />;
    if (title.includes('Output')) return <Terminal className="w-4 h-4" />;
    if (title.includes('Target')) return <Code className="w-4 h-4" />;
    if (title.includes('Machine')) return <Cpu className="w-4 h-4" />;
    return <Code className="w-4 h-4" />;
  };

  const getEditorBg = () => {
    if (isDark) {
      return title.includes('Source')
        ? 'bg-gradient-to-br from-gray-900/40 to-gray-950/20'
        : 'bg-gradient-to-br from-gray-900/30 to-gray-950/15';
    } else {
      return 'bg-white';
    }
  };

  const getTextColor = () => (isDark ? 'text-white' : 'text-black');
  const getBorderColor = () => (isDark ? 'border-gray-700/50' : 'border-gray-300/50');
  const get_bg_color = () => (isDark ? '' : 'bg-white');

  // === Machine code specific styling ===
  const getMachineCodeBg = () => {
    return isDark ? '#111827' : '#ffffff';
  };

  const getMachineCodeTextColor = () => {
    return isDark ? '#f9fafb' : '#111827';
  };

  const getMachineCodeHeaderBg = () => {
    return isDark ? 'bg-gray-800/50' : 'bg-gray-100';
  };

  const getMachineCodeHeaderText = () => {
    return isDark ? 'text-gray-300' : 'text-gray-700';
  };

  const getMonacoLanguage = () => {
    if (title.includes('Source')) return 'baiscript';
    return 'plaintext';
  };

  const handleEditorWillMount = (monaco: any) => {
    // Register BaiScript as a custom language
    monaco.languages.register({ id: 'baiscript' });

    // Define BaiScript syntax highlighting
    monaco.languages.setMonarchTokensProvider('baiscript', {
      keywords: [
        'ENTEGER', 'KUAN', 'CHAROT', 'PRENT', 'LOOP', 'IF', 'ELSE', 'WHILE', 'FOR',
        'FUNCTION', 'RETURN', 'TRUE', 'FALSE', 'NULL'
      ],
      operators: [
        '=', '>', '<', '!', '~', '?', ':', '==', '<=', '>=', '!=',
        '&&', '||', '++', '--', '+', '-', '*', '/', '&', '|', '^', '%',
        '<<', '>>', '+=', '-=', '*=', '/='
      ],
      symbols: /[=><!~?:&|+\-*\/\^%]+/,
      escapes: /\\(?:[abfnrtv\\"']|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,

      tokenizer: {
        root: [
          // Keywords (all uppercase based on your grammar)
          [/[A-Z][A-Z_]*/, {
            cases: {
              '@keywords': 'keyword',
              '@default': 'type.identifier'
            }
          }],

          // Identifiers (lowercase)
          [/[a-z_$][\w$]*/, 'identifier'],

          // Numbers
          [/\d+/, 'number'],

          // Strings
          [/"([^"\\]|\\.)*$/, 'string.invalid'],  // non-terminated string
          [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],

          // Characters
          [/'[^\\']'/, 'string'],
          [/(')(@escapes)(')/, ['string', 'string.escape', 'string']],
          [/'/, 'string.invalid'],

          // Comments
          [/\/\/.*$/, 'comment'],

          // Delimiters and operators
          [/[{}()\[\]]/, '@brackets'],
          [/[;,.]/, 'delimiter'],
          [/@symbols/, {
            cases: {
              '@operators': 'operator',
              '@default': ''
            }
          }],

          // Whitespace
          { include: '@whitespace' }
        ],

        string: [
          [/[^\\"]+/, 'string'],
          [/@escapes/, 'string.escape'],
          [/\\./, 'string.escape.invalid'],
          [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
        ],

        whitespace: [
          [/[ \t\r\n]+/, 'white'],
          [/\/\*/, 'comment', '@comment'],
          [/\/\/.*$/, 'comment'],
        ],

        comment: [
          [/[^\/*]+/, 'comment'],
          [/\/\*/, 'comment', '@push'],
          ["\\*/", 'comment', '@pop'],
          [/[\/*]/, 'comment']
        ],
      }
    });

    // Set language configuration for BaiScript
    monaco.languages.setLanguageConfiguration('baiscript', {
      comments: {
        lineComment: '//',
        blockComment: ['/*', '*/']
      },
      brackets: [
        ['(', ')'],
        ['[', ']'],
        ['{', '}']
      ],
      autoClosingPairs: [
        { open: '(', close: ')' },
        { open: '[', close: ']' },
        { open: '{', close: '}' },
        { open: '"', close: '"' },
        { open: "'", close: "'" }
      ],
      surroundingPairs: [
        { open: '(', close: ')' },
        { open: '[', close: ']' },
        { open: '{', close: '}' },
        { open: '"', close: '"' },
        { open: "'", close: "'" }
      ]
    });

    // Define custom themes
    monaco.editor.defineTheme('bai-dark', {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: 'keyword', foreground: 'C586C0', fontStyle: 'bold' },
        { token: 'type.identifier', foreground: '4EC9B0' },
        { token: 'identifier', foreground: '9CDCFE' },
        { token: 'string', foreground: 'CE9178' },
        { token: 'number', foreground: 'B5CEA8' },
        { token: 'comment', foreground: '6A9955' },
        { token: 'operator', foreground: 'D4D4D4' },
        { token: 'delimiter', foreground: 'D4D4D4' },
      ],
      colors: {
        'editor.background': title.includes('Source') ? '#1f2937' : '#111827',
        'editor.foreground': '#f9fafb',
        'editor.lineHighlightBackground': '#374151',
        'editor.selectionBackground': '#4b5563',
        'editorLineNumber.foreground': '#6b7280',
        'editorLineNumber.activeForeground': '#d1d5db',
        'editorCursor.foreground': '#f9fafb',
        'editorIndentGuide.background': '#374151',
        'editorIndentGuide.activeBackground': '#6b7280',
      }
    });

    monaco.editor.defineTheme('bai-light', {
      base: 'vs',
      inherit: true,
      rules: [
        { token: 'keyword', foreground: '0000FF', fontStyle: 'bold' },
        { token: 'type.identifier', foreground: '267F99' },
        { token: 'identifier', foreground: '001080' },
        { token: 'string', foreground: 'A31515' },
        { token: 'number', foreground: '098658' },
        { token: 'comment', foreground: '008000' },
        { token: 'operator', foreground: '000000' },
        { token: 'delimiter', foreground: '000000' },
      ],
      colors: {
        'editor.background': '#ffffff',
        'editor.foreground': '#111827',
        'editor.lineHighlightBackground': '#f3f4f6',
        'editor.selectionBackground': '#d1d5db',
        'editorLineNumber.foreground': '#9ca3af',
        'editorLineNumber.activeForeground': '#374151',
        'editorCursor.foreground': '#111827',
        'editorIndentGuide.background': '#e5e7eb',
        'editorIndentGuide.activeBackground': '#9ca3af',
      }
    });
  };

  const handleChange = (newVal: string | undefined) => {
    if (title.includes('Source') && onChange && newVal !== undefined) {
      onChange(newVal);
    }
  };

  // === Machine code rendering ===
  const renderMachineCodeSections = () => {
    if (isMachineCode) {
      const machineValue = value as MachineCodeValue;
      return (
        <div className="flex-1 flex overflow-hidden">
          {/* Assembly */}
          <div className="w-3/10 flex flex-col border-r border-gray-600/30">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${getMachineCodeHeaderBg()} ${getMachineCodeHeaderText()}`}>
              ASSEMBLY CODE
            </div>
            <div
              className="flex-1 overflow-auto pl-4 pr-4 text-sm font-mono"
              style={{
                lineHeight: '1.6',
                tabSize: 2,
                whiteSpace: 'pre',
                backgroundColor: getMachineCodeBg(),
                color: getMachineCodeTextColor()
              }}
            >
              {machineValue.assembly || 'No assembly generated'}
            </div>
          </div>

          {/* Binary */}
          <div className="w-5/10 flex flex-col border-r border-gray-600/30">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${getMachineCodeHeaderBg()} ${getMachineCodeHeaderText()}`}>
              BINARY CODE
            </div>
            <div
              className="flex-1 overflow-auto pl-4 pr-4 text-sm font-mono"
              style={{
                lineHeight: '1.6',
                tabSize: 2,
                whiteSpace: 'pre',
                backgroundColor: getMachineCodeBg(),
                color: getMachineCodeTextColor()
              }}
            >
              {machineValue.binary || 'No binary generated'}
            </div>
          </div>

          {/* Hex */}
          <div className="w-2/10 flex flex-col">
            <div className={`px-4 py-2 font-semibold text-sm border-b border-gray-600/30 ${getMachineCodeHeaderBg()} ${getMachineCodeHeaderText()}`}>
              HEX CODE
            </div>
            <div
              className="flex-1 overflow-auto pl-4 pr-4 text-sm font-mono"
              style={{
                lineHeight: '1.6',
                tabSize: 2,
                whiteSpace: 'pre',
                backgroundColor: getMachineCodeBg(),
                color: getMachineCodeTextColor()
              }}
            >
              {machineValue.hex || 'No hex generated'}
            </div>
          </div>
        </div>
      );
    }

    // === Monaco editor for Source / Output / Target ===
    const editorText = typeof value === 'string' ? value : '';

    return (
      <div className={`flex-1 overflow-hidden ${!title.includes('Source') ? 'select-none' : ''}`}>
        <MonacoEditor
          height="100%"
          language={getMonacoLanguage()}
          value={editorText}
          onChange={handleChange}
          beforeMount={handleEditorWillMount}
          theme={isDark ? 'bai-dark' : 'bai-light'}
          options={{
            readOnly: !editable || readOnly || !title.includes('Source'),
            minimap: { enabled: false },
            automaticLayout: true,
            fontSize: 14,
            lineHeight: 1.6,
            padding: { top: 10, bottom: 10 },
            wordWrap: 'on',
            scrollBeyondLastLine: false,
            lineNumbers: title.includes('Source') ? 'on' : 'off',
            renderLineHighlight: title.includes('Source') ? 'all' : 'none',
            selectionHighlight: title.includes('Source'),
            occurrencesHighlight: 'off',
            matchBrackets: title.includes('Source') ? 'always' : 'never',
            folding: title.includes('Source'),
            foldingHighlight: title.includes('Source'),
            showFoldingControls: 'mouseover',
            cursorStyle: title.includes('Source') ? 'line' : 'block',
          }}
          loading={
            <div className={`flex items-center justify-center h-full ${getEditorBg()} ${getTextColor()}`}>
              <div className="animate-pulse">Loading editor...</div>
            </div>
          }
        />
      </div>
    );
  };

  const getMachineCodeLineCounts = () => {
    if (isMachineCode) {
      const machineValue = value as MachineCodeValue;
      return {
        assembly: machineValue.assembly?.split('\n').length || 0,
        binary: machineValue.binary?.split('\n').length || 0,
        hex: machineValue.hex?.split('\n').length || 0
      };
    }
    return { assembly: 0, binary: 0, hex: 0 };
  };

  const lineCounts = getMachineCodeLineCounts();

  const getCharacterCount = () => {
    if (isMachineCode) {
      const machineValue = value as MachineCodeValue;
      return (machineValue.assembly?.length || 0) + (machineValue.binary?.length || 0) + (machineValue.hex?.length || 0);
    }
    return typeof value === 'string' ? value.length : 0;
  };

  const getLineCount = () => {
    if (isMachineCode) {
      return `Assembly: ${lineCounts.assembly} lines | Binary: ${lineCounts.binary} lines | Hex: ${lineCounts.hex} lines`;
    }
    return typeof value === 'string' ? `${value.split('\n').length} lines` : '0 lines';
  };

  const displayedLanguage = isMachineCode ? 'Assembly/Binary/Hex' : language;

  return (
    <div className={`flex flex-col rounded-xl border ${getBorderColor()} backdrop-blur-sm overflow-hidden transition-colors shadow-md hover:shadow-lg ${get_bg_color()}`}>
      {/* Header */}
      <div className={`flex items-center justify-between px-4 py-3 transition-colors ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <div className="flex items-center gap-2">
          {getIcon()}
          <div>
            <h3 className="text-sm font-semibold">{title}</h3>
            <p className={`text-xs transition-colors ${isDark ? 'text-gray-400' : 'text-gray-600'}`}>{subtitle}</p>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
            {displayedLanguage}
          </span>
          <div className="flex gap-1.5">
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
            <div className={`w-3 h-3 rounded-full transition-colors ${isDark ? 'bg-gray-600/50' : 'bg-gray-400/50'}`}></div>
          </div>
        </div>
      </div>

      {/* Editor Area */}
      {renderMachineCodeSections()}

      {/* Footer */}
      <div className={`flex items-center justify-between px-4 py-2 transition-colors ${isDark ? 'border-gray-800/50 bg-black/30' : 'border-gray-300/70 bg-gray-200'}`}>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {getLineCount()}
        </span>
        <span className={`text-xs transition-colors ${isDark ? 'text-gray-500' : 'text-gray-600'}`}>
          {getCharacterCount()} characters
        </span>
      </div>
    </div>
  );
}