import { CodeBlock } from "./CodeBlock";

interface DocumentationContentProps {
    activeSection: string;
    isDarkMode: boolean;
}

export function DocumentationContent({ activeSection, isDarkMode }: DocumentationContentProps) {
    const textColor = isDarkMode ? "text-white" : "text-gray-900";
    const subTextColor = isDarkMode ? "text-gray-400" : "text-gray-600";

    return (
        <main className="flex-1 p-8 max-w-4xl">
            {activeSection === 'introduction' && <IntroductionSection textColor={textColor} subTextColor={subTextColor} />}
            {activeSection === 'datatypes' && <DatatypesSection textColor={textColor} subTextColor={subTextColor} />}
            {activeSection === 'variables' && <DeclarationsSection textColor={textColor} subTextColor={subTextColor} />}
            {activeSection === 'assignments' && <AssignmentsSection textColor={textColor} subTextColor={subTextColor} />}
            {activeSection === 'expressions' && <ExpressionsSection textColor={textColor} subTextColor={subTextColor} />}
            {activeSection === 'io' && <IOSection textColor={textColor} subTextColor={subTextColor} />}
        </main>
    );
}

/* ---------------- INTRODUCTION ---------------- */
function IntroductionSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <div>
                <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>
                    Introduction
                </h1>
                <p className={subTextColor}>
                    Welcome to BaiScript, a modern multi-stage compiler environment designed for efficient code execution.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>
                    What is BaiScript?
                </h2>
                <p className={subTextColor}>
                    BaiScript is a programming language inspired by the Bisaya language and the word "Bai", which means friend or brother. It features unique keywords such as ENTEGER, CHAROT, and PRENT, giving it a distinct syntax and cultural flavor.
                </p>
                <p className={subTextColor}>
                    BaiScript accepts source code in plain text format and translates it into MIPS 64 assembly code, also in text format, allowing users to run and experiment with the generated code using the EduMips64 software.
                </p>
            </section>

            <section className="space-y-4">
                <h3 className={`text-xl font-semibold ${textColor}`}>Your First Program</h3>
                <p className={subTextColor}>
                    Let's start with a simple "Hello, World!" program:
                </p>
                <CodeBlock code={`PRENT "Hello, world"!`} language="baiscript" />
            </section>
        </div>
    );
}

/* ---------------- DATATYPES ---------------- */
function DatatypesSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <div>
                <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>Datatypes</h1>
                <p className={subTextColor}>
                    BaiScript supports several fundamental datatypes for storing and manipulating different kinds of data. It also allows general-purpose type inference using the <code>KUAN</code> keyword.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Primitive Types</h2>

                <div className="space-y-6">
                    <div>
                        <h3 className={`text-xl font-semibold mb-3 ${textColor}`}>Integer (ENTEGER)</h3>
                        <p className={`${subTextColor} mb-3`}>
                            Represents whole numbers. Use the <code>ENTEGER</code> keyword to declare integer variables.
                        </p>
                        <CodeBlock code={`ENTEGER a = 1!
ENTEGER b = -10!
ENTEGER c = 0!`} language="baiscript" />
                    </div>

                    <div>
                        <h3 className={`text-xl font-semibold mb-3 ${textColor}`}>Character (CHAROT)</h3>
                        <p className={`${subTextColor} mb-3`}>
                            Represents a single character. Use the <code>CHAROT</code> keyword to declare character variables.
                        </p>
                        <CodeBlock code={`CHAROT letter = 'a'!
CHAROT symbol = '!'!`} language="baiscript" />
                    </div>

                    <div>
                        <h3 className={`text-xl font-semibold mb-3 ${textColor}`}>General-Purpose (KUAN)</h3>
                        <p className={`${subTextColor} mb-3`}>
                            <code>KUAN</code> can hold either an <code>ENTEGER</code> or <code>CHAROT</code>. Its type is determined by the leftmost value assigned.
                        </p>
                        <CodeBlock code={`KUAN x = 42!       // becomes ENTEGER
KUAN y = 'b'!      // becomes CHAROT
KUAN z = 0!        // becomes ENTEGER`} language="baiscript" />
                    </div>
                </div>
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Type Inference</h2>
                <p className={subTextColor}>
                    BaiScript can automatically infer types using <code>KUAN</code>, allowing flexible and concise code.
                </p>
                <CodeBlock code={`KUAN num = 100!       // inferred as ENTEGER
KUAN char = 'x'!     // inferred as CHAROT`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Summary</h2>
                <p className={subTextColor}>
                    - <code>ENTEGER</code>: whole numbers <br />
                    - <code>CHAROT</code>: single characters <br />
                    - <code>KUAN</code>: general-purpose datatype, inferred from the leftmost value
                </p>
            </section>
        </div>
    );
}

/* ---------------- VARIABLES ---------------- */
function DeclarationsSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>Variables and Declarations</h1>
            <p className={subTextColor}>
                Variables are declared using a datatype followed by one or more identifiers. Multiple declarations are separated by commas.
            </p>

            <div>
                <h3 className={`text-xl font-semibold mb-1 ${textColor}`}>Examples</h3>
                <CodeBlock code={`ENTEGER x, y, z!
CHAROT letter, symbol!
KUAN a, b!`} language="baiscript" />
            </div>

            <div>
                <h3 className={`text-xl font-semibold mb-1 ${textColor}`}>Declarations with initialization</h3>
                <CodeBlock code={`ENTEGER x = 10!, y = 20!
CHAROT letter = 'a'!
KUAN a = 5!, b = 'b'!`} language="baiscript" />
            </div>
        </div>
    );
}

/* ---------------- ASSIGNMENTS ---------------- */
function AssignmentsSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>Assignments</h1>
            <p className={subTextColor}>
                Values can be assigned to variables after declaration using assignment operators. Supported operators include <code>=</code>, <code>+=</code>, <code>-=</code>, <code>*=</code>, <code>/=</code>.
            </p>

            <div>
                <h3 className={`text-xl font-semibold mb-1 ${textColor}`}>Examples</h3>
                <CodeBlock code={`x = 5!
y += 10!
z -= 3!
a *= 2!
b /= 4!`} language="baiscript" />
            </div>
        </div>
    );
}

/* ---------------- EXPRESSIONS ---------------- */
function ExpressionsSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>Expressions and Operators</h1>
            <p className={subTextColor}>
                BaiScript supports arithmetic expressions and both prefix (unary) and postfix operators. Expressions can be combined with <code>+</code>, <code>-</code>, <code>*</code>, <code>/</code>, and variables or literals.
            </p>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Arithmetic Expressions</h2>
                <CodeBlock code={`ENTEGER a = 10!
ENTEGER b = 5!
ENTEGER sum = a + b!
ENTEGER diff = a - b!
ENTEGER prod = a * b!
ENTEGER quot = a / b!`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Prefix (Unary) Operators</h2>
                <p className={subTextColor}>
                    Unary operators include <code>+</code>, <code>-</code>, <code>++</code>, and <code>--</code> when placed **before a variable or expression**.
                </p>
                <CodeBlock code={`++a!      // increment before using
--b!      // decrement before using
+a!       // unary plus (no effect)
-b!       // unary minus (negates value)`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Postfix Operators</h2>
                <p className={subTextColor}>
                    Postfix operators include <code>++</code> and <code>--</code> when placed **after a variable**.
                </p>
                <CodeBlock code={`a++!      // increment after using
b--!      // decrement after using`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Combined Expressions</h2>
                <CodeBlock code={`ENTEGER a = 5!
ENTEGER b = 3!
ENTEGER c = ++a + b--!  // a is incremented first, b used then decremented
ENTEGER d = -c * 2!`
                } language="baiscript" />
            </section>
        </div>
    );
}

/* ---------------- OUTPUT ---------------- */
function IOSection({ textColor, subTextColor }: { textColor: string; subTextColor: string }) {
    return (
        <div className="space-y-6">
            <div>
                <h1 className={`text-3xl font-bold mb-2 ${textColor}`}>Output with PRENT</h1>
                <p className={subTextColor}>
                    BaiScript does not support user input. You can output values using the <code>PRENT</code> keyword. <code>PRENT</code> can display literals, variables, or expressions directly to the console. Multiple items can be concatenated using commas.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Printing Literals and Variables</h2>
                <p className={subTextColor}>
                    Use <code>PRENT</code> to display values. You can print integers, characters, or <code>KUAN</code> variables:
                </p>
                <CodeBlock code={`ENTEGER x = 42!
CHAROT c = 'a'!
KUAN a = 100!

PRENT x!     // Output: 42
PRENT c!     // Output: a
PRENT a!     // Output: 100`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Using KUAN in Expressions</h2>
                <p className={subTextColor}>
                    If a <code>KUAN</code> variable holds a <code>CHAROT</code> value, arithmetic with numbers treats it as its ASCII value:
                </p>
                <CodeBlock code={`KUAN a = 'a'!
PRENT a + 1!       // Output: b  (ASCII 'a' + 1 = 'b')

KUAN b = 1!
PRENT b + 'a'!     // Output: 98  (1 + ASCII 'a' = 1 + 97)`} language="baiscript" />
                <p className={subTextColor}>
                    In expressions, <code>PRENT</code> automatically converts the result to a character if the leftmost operand is a <code>CHAROT</code>. Otherwise, it prints the numeric value.
                </p>
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Printing Multiple Values</h2>
                <p className={subTextColor}>
                    You can print multiple items separated by commas for concatenation:
                </p>
                <CodeBlock code={`KUAN a = 1!
KUAN b = 'b'!

PRENT "Hello", " ", "World"!      // Output: Hello World
PRENT a, " + ", b, " = ", a + b!  // Output: 1 + b = 99`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className={`text-2xl font-bold ${textColor}`}>Best Practices</h2>
                <div className={`p-4 border rounded-lg ${textColor.includes('white') ? 'bg-green-900/20 border-green-800' : 'bg-green-50 border-green-200'}`}>
                    <ul className={`space-y-2 list-disc list-inside ${textColor.includes('white') ? 'text-gray-300' : 'text-gray-700'}`}>
                        <li>Use <code>PRENT</code> for all output; BaiScript does not support input operations.</li>
                        <li>When using <code>KUAN</code> variables, remember type inference rules affect arithmetic operations.</li>
                        <li>Use commas <code>,</code> to concatenate multiple items in a single <code>PRENT</code> statement.</li>
                        <li>Wrap expressions in parentheses if needed for clarity when combining multiple values.</li>
                    </ul>
                </div>
            </section>
        </div>
    );
}