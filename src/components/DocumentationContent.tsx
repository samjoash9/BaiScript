import { CodeBlock } from "./CodeBlock";

interface DocumentationContentProps {
    activeSection: string;
    isDarkMode: boolean;
}

export function DocumentationContent({ activeSection, isDarkMode }: DocumentationContentProps) {
    return (
        <main className="flex-1 p-8 max-w-4xl">
            {activeSection === 'introduction' && <IntroductionSection />}
            {activeSection === 'datatypes' && <DatatypesSection />}
            {activeSection === 'variables' && <DeclarationsSection />}
            {activeSection === 'assignments' && <AssignmentsSection />}
            {activeSection === 'expressions' && <ExpressionsSection />}
            {activeSection === 'io' && <IOSection />}
        </main>
    );
}

/* ---------------- INTRODUCTION ---------------- */
function IntroductionSection() {
    return (
        <div className="space-y-6">
            <div>
                <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">
                    Introduction
                </h1>
                <p className="text-gray-600 dark:text-gray-400">
                    Welcome to BaiScript, a modern multi-stage compiler environment designed for efficient code execution.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
                    What is BaiScript?
                </h2>
                <p className="text-gray-600 dark:text-gray-400">
                    BaiScript is a programming language inspired by the Bisaya language and the word “Bai”, which means friend or brother. It features unique keywords such as ENTEGER, CHAROT, and PRENT, giving it a distinct syntax and cultural flavor.
                </p>
                <p className="text-gray-600 dark:text-gray-400">
                    BaiScript accepts source code in plain text format and translates it into MIPS 64 assembly code, also in text format, allowing users to run and experiment with the generated code using the EduMips64 software.
                </p>
            </section>

            <section className="space-y-4">
                <h3 className="text-xl font-semibold text-gray-900 dark:text-white">Your First Program</h3>
                <p className="text-gray-600 dark:text-gray-400">
                    Let's start with a simple "Hello, World!" program:
                </p>
                <CodeBlock code={`PRENT "Hello, world"!`} language="baiscript" />
            </section>
        </div>
    );
}

/* ---------------- DATATYPES ---------------- */
function DatatypesSection() {
    return (
        <div className="space-y-6">
            <div>
                <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">Datatypes</h1>
                <p className="text-gray-600 dark:text-gray-400">
                    BaiScript supports several fundamental datatypes for storing and manipulating different kinds of data. It also allows general-purpose type inference using the <code>KUAN</code> keyword.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Primitive Types</h2>

                <div className="space-y-6">
                    <div>
                        <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-3">Integer (ENTEGER)</h3>
                        <p className="text-gray-600 dark:text-gray-400 mb-3">
                            Represents whole numbers. Use the <code>ENTEGER</code> keyword to declare integer variables.
                        </p>
                        <CodeBlock code={`ENTEGER a = 1!;
ENTEGER b = -10!;
ENTEGER c = 0!;`} language="baiscript" />
                    </div>

                    <div>
                        <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-3">Character (CHAROT)</h3>
                        <p className="text-gray-600 dark:text-gray-400 mb-3">
                            Represents a single character. Use the <code>CHAROT</code> keyword to declare character variables.
                        </p>
                        <CodeBlock code={`CHAROT letter = 'a'!;
CHAROT symbol = '!';`} language="baiscript" />
                    </div>

                    <div>
                        <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-3">General-Purpose (KUAN)</h3>
                        <p className="text-gray-600 dark:text-gray-400 mb-3">
                            <code>KUAN</code> can hold either an <code>ENTEGER</code> or <code>CHAROT</code>. Its type is determined by the leftmost value assigned.
                        </p>
                        <CodeBlock code={`KUAN x = 42!       // becomes ENTEGER
KUAN y = 'b'!      // becomes CHAROT
KUAN z = 0!        // becomes ENTEGER`} language="baiscript" />
                    </div>
                </div>
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Type Inference</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    BaiScript can automatically infer types using <code>KUAN</code>, allowing flexible and concise code.
                </p>
                <CodeBlock code={`KUAN num = 100!       // inferred as ENTEGER
KUAN char = 'x'!     // inferred as CHAROT`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Summary</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    - <code>ENTEGER</code>: whole numbers <br />
                    - <code>CHAROT</code>: single characters <br />
                    - <code>KUAN</code>: general-purpose datatype, inferred from the leftmost value
                </p>
            </section>
        </div>
    );
}

/* ---------------- VARIABLES ---------------- */
function DeclarationsSection() {
    return (
        <div className="space-y-6">
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">Variables and Declarations</h1>
            <p className="text-gray-600 dark:text-gray-400">
                Variables are declared using a datatype followed by one or more identifiers. Multiple declarations are separated by commas.
            </p>

            <div>
                <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-1">Examples</h3>
                <CodeBlock code={`ENTEGER x, y, z!
CHAROT letter, symbol!
KUAN a, b!`} language="baiscript" />
            </div>

            <div>
                <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-1">Declarations with initialization</h3>
                <CodeBlock code={`ENTEGER x = 10!, y = 20!
CHAROT letter = 'a'!
KUAN a = 5!, b = 'b'!`} language="baiscript" />
            </div>
        </div>
    );
}

/* ---------------- ASSIGNMENTS ---------------- */
function AssignmentsSection() {
    return (
        <div className="space-y-6">
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">Assignments</h1>
            <p className="text-gray-600 dark:text-gray-400">
                Values can be assigned to variables after declaration using assignment operators. Supported operators include <code>=</code>, <code>+=</code>, <code>-=</code>, <code>*=</code>, <code>/=</code>.
            </p>

            <div>
                <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-1">Examples</h3>
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
function ExpressionsSection() {
    return (
        <div className="space-y-6">
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">Expressions and Operators</h1>
            <p className="text-gray-600 dark:text-gray-400">
                BaiScript supports arithmetic expressions and both prefix (unary) and postfix operators. Expressions can be combined with <code>+</code>, <code>-</code>, <code>*</code>, <code>/</code>, and variables or literals.
            </p>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Arithmetic Expressions</h2>
                <CodeBlock code={`ENTEGER a = 10!
ENTEGER b = 5!
ENTEGER sum = a + b!
ENTEGER diff = a - b!
ENTEGER prod = a * b!
ENTEGER quot = a / b!`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Prefix (Unary) Operators</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    Unary operators include <code>+</code>, <code>-</code>, <code>++</code>, and <code>--</code> when placed **before a variable or expression**.
                </p>
                <CodeBlock code={`++a!      // increment before using
--b!      // decrement before using
+a!       // unary plus (no effect)
-b!       // unary minus (negates value)`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Postfix Operators</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    Postfix operators include <code>++</code> and <code>--</code> when placed **after a variable**.
                </p>
                <CodeBlock code={`a++!      // increment after using
b--!      // decrement after using`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Combined Expressions</h2>
                <CodeBlock code={`ENTEGER a = 5!
ENTEGER b = 3!
ENTEGER c = ++a + b--!  // a is incremented first, b used then decremented
ENTEGER d = -c * 2!`} language="baiscript" />
            </section>
        </div>
    );
}

/* ---------------- OUTPUT ---------------- */
function IOSection() {
    return (
        <div className="space-y-6">
            <div>
                <h1 className="text-3xl font-bold text-gray-900 dark:text-white mb-2">Output with PRENT</h1>
                <p className="text-gray-600 dark:text-gray-400">
                    BaiScript does not support user input. You can output values using the <code>PRENT</code> keyword. <code>PRENT</code> can display literals, variables, or expressions directly to the console. Multiple items can be concatenated using commas.
                </p>
            </div>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Printing Literals and Variables</h2>
                <p className="text-gray-600 dark:text-gray-400">
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
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Using KUAN in Expressions</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    If a <code>KUAN</code> variable holds a <code>CHAROT</code> value, arithmetic with numbers treats it as its ASCII value:
                </p>
                <CodeBlock code={`KUAN a = 'a'!
PRENT a + 1!       // Output: b  (ASCII 'a' + 1 = 'b')

KUAN b = 1!
PRENT b + 'a'!     // Output: 98  (1 + ASCII 'a' = 1 + 97)`} language="baiscript" />
                <p className="text-gray-600 dark:text-gray-400">
                    In expressions, <code>PRENT</code> automatically converts the result to a character if the leftmost operand is a <code>CHAROT</code>. Otherwise, it prints the numeric value.
                </p>
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Printing Multiple Values</h2>
                <p className="text-gray-600 dark:text-gray-400">
                    You can print multiple items separated by commas for concatenation:
                </p>
                <CodeBlock code={`KUAN a = 1!
KUAN b = 'b'!

PRENT "Hello", " ", "World"!      // Output: Hello World
PRENT a, " + ", b, " = ", a + b!  // Output: 1 + b = 99`} language="baiscript" />
            </section>

            <section className="space-y-4">
                <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Best Practices</h2>
                <div className="p-4 bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg">
                    <ul className="space-y-2 text-gray-700 dark:text-gray-300 list-disc list-inside">
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
