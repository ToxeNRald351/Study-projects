<?php
    /*
    *   Autor: Anton Havlovskyi
    *   Login: xhavlo01
    */
    
    define("MANUAL", "    This is a parsing script.\n    Use it to parse \".IPPcode22\" code into XML.
    This script will use stdin and stdout as in/out streams,
    however you can change them by setting any other prefereble sources
    as \"do_some_magic\" function parameters.
    All errors go to stderr.
    Enjoy!\n");

    class IOStream {
        public $input;
        public $output;
        public $inpath;
        public $outpath;
        
        public $order;
        
        public $line;
        public $arg_pos;

        public $options;
        public $statistics = array(
            "loc" => array(false),
            "comments" => array(false),
            "labels" => array(false),
            "jumps" => array(false),
            "fwjumps" => array(false),
            "backjumps" => array(false),
            "badjumps" => array(false)
        );
        public $labels = array();
        public $jumps = array();
        
        const OPCODES = array(
            "MOVE"          => array(2, "var", "symb"),
            "CREATEFRAME"   => array(0),
            "PUSHFRAME"     => array(0),
            "POPFRAME"      => array(0),
            "DEFVAR"        => array(1, "var"),
            "CALL"          => array(1, "label"),
            "RETURN"        => array(0),
            "PUSHS"         => array(1, "symb"),
            "POPS"          => array(1, "var"),
            "ADD"           => array(3, "var", "symb", "symb"),
            "SUB"           => array(3, "var", "symb", "symb"),
            "MUL"           => array(3, "var", "symb", "symb"),
            "IDIV"          => array(3, "var", "symb", "symb"),
            "LT"            => array(3, "var", "symb", "symb"),
            "GT"            => array(3, "var", "symb", "symb"),
            "EQ"            => array(3, "var", "symb", "symb"),
            "AND"           => array(3, "var", "symb", "symb"),
            "OR"            => array(3, "var", "symb", "symb"),
            "NOT"           => array(3, "var", "symb", "symb"),
            "INT2CHAR"      => array(2, "var", "symb"),
            "STRI2INT"      => array(3, "var", "symb", "symb"),
            "READ"          => array(2, "var", "type"),
            "WRITE"         => array(1, "symb"),
            "CONCAT"        => array(3, "var", "symb", "symb"),
            "STRLEN"        => array(2, "var", "symb"),
            "GETCHAR"       => array(3, "var", "symb", "symb"),
            "SETCHAR"       => array(3, "var", "symb", "symb"),
            "TYPE"          => array(2, "var", "symb"),
            "LABEL"         => array(1, "label"),
            "JUMP"          => array(1, "label"),
            "JUMPIFEQ"      => array(3, "label", "symb", "symb"),
            "JUMPIFNEQ"     => array(3, "label", "symb", "symb"),
            "EXIT"          => array(1, "symb"),
            "DPRINT"        => array(1, "symb"),
            "BREAK"         => array(0)
        );
        const varnlabelpattern = "/[^a-z|^0-9|^_|^-|^\$|^&|^%|^\*|^!|^\?]/i";

        function __construct($inpath, $outpath, $options) {
            $this -> input = fopen($inpath, "r") or exit(11);
            $this -> output = fopen($outpath, "w+") or exit(12);
            $this -> order = 1;
            $this -> line = 0;
            $this -> arg_pos = 0;

            $this -> options = $options;
            foreach ($this -> options as $filename => $parameters_array) {
                foreach ($parameters_array as $parameter) {
                    $this -> statistics[$parameter][0] = true;
                    $this -> statistics[$parameter][] = 0;
                }
            }
        }

        function headctrl() {
            while (!feof($this -> input)){
                $this -> line++;
                if (!$strvar = fgets($this -> input)) { break; };
                $strvar = $this -> rmcoms($strvar);
                $strvar = trim($strvar);
                if (!strcmp($strvar, ".IPPcode22")) {
                    return;
                }
            }
            fwrite(STDERR, "Missing \".IPPcode22\" head!\n");
            exit (21);
        }

        function opproc() {
            if (!feof($this -> input)) {
                //get line from file
                $strvar = fgets($this -> input);
                //increment line counter
                $this -> line++;
                //delete commentaries, format line
                $strvar = $this -> rmcoms($strvar);
                $strvar = trim($strvar);
                if (strlen($strvar)) {
                    //get all expressions from line, max 4
                    $result = sscanf($strvar, "%s %s %s %s", $array[0], $array[1], $array[2], $array[3]);
                    $value = self::OPCODES[$array[0]][0] ?? notfound_22();
                    if ($value === $result - 1) {
                        //OPCODE ok, increment counter if parameter set
                        if ($this -> statistics["loc"][0]) {
                            $this -> statistics["loc"][1]++;
                        }
                        //write
                        fwrite($this -> output, "\t<instruction order=\"" . $this -> order . "\" opcode=\"" . $array[0] . "\">\n");
                        for ($i = 1; $i < $result; $i++) {
                            //some preparations
                            //arg counter set
                            $this -> arg_pos = $i;
                            //type control
                            $arg_type = $this -> deftype($array[$i]);
                            $reqv_type = self::OPCODES[$array[0]][$i];
                            //symb is a combination of constants and variables
                            if (!strcmp($reqv_type, "symb")) {
                                $reqv_type = "var|int|string|bool|nil";
                            }
                            if (strpos($reqv_type, $arg_type) === false) {
                                fwrite(STDERR, "Types don't match!\n");
                                fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                                exit(23);
                            }
                            //format control
                            //if type is of {"int", "bool", "string", "nil"}, cut off *type*@ part
                            if (strpos("int|bool|string|nil", $arg_type) !== false) {
                                $array[$i] = substr($array[$i], strpos($array[$i], "@") + 1);
                            }
                            //content control
                            //content must correspond to its type
                            $this -> contentctrl($array[$i], $arg_type);
                            //if string, convert certain symbols into html code
                            if (!strcmp($arg_type, "string")) {
                                $array[$i] = $this -> symbconvert($array[$i]);
                            }
                            //function to gather labels and jumps stats goes here
                            $this -> gatherstats($array);
                            //all ok, write
                            fwrite($this -> output, "\t\t<arg$i type=\"$arg_type\">$array[$i]</arg$i>\n");
                        }
                        fwrite($this -> output, "\t</instruction>\n");
                        $this -> order++;
                        return true;
                    }
                    fwrite(STDERR, "ERROR: OPCODE expects more or less arguments!\n");
                    fwrite(STDERR, "Error occured here: line " . $this -> line . ".\n");
                    exit(23);
                }
                return true;
            }
            return false;
        }

        function gatherstats($array) {
            //if set, increment labels counter
            if ($this -> statistics["labels"][0] and !strcmp($array[0], "LABEL")) {
                $this -> statistics["labels"][1]++;
            }
            //if set, increment jumps counter
            if ($this -> statistics["jumps"][0] and 
                (strpos($array[0], "JUMP") !== false or 
                !strcmp($array[0], "CALL") or !strcmp($array[0], "RETURN"))) {
                $this -> statistics["jumps"][1]++;
            }
            //if there's set a directional jump counter, gather labels and jumps data
            //we'll use it later
            if ($this -> statistics["fwjumps"][0] or $this -> statistics["backjumps"][0] or $this -> statistics["badjumps"][0]) {
                if (!strcmp($array[0], "LABEL")) {
                    $this -> labels += array($array[1] => $this -> line);
                }
                elseif (strpos($array[0], "JUMP") !== false or 
                       !strcmp($array[0], "CALL")) {
                    $this -> jumps += array($array[1] => $this -> line);
                }
            }
        }

        function notfound_22() {
            fwrite(STDERR, "ERROR: Unknown opcode!\n");
            fwrite(STDERR, "Error occured here: line " . $this -> line . ".\n");
            exit(22);
        }
        
        function symbconvert($string) {
            foreach (array("&" => "&amp;", "<" => "&lt;", ">" => "&gt;") as $symbol => $replacement) {
                $pos = strpos($string, $symbol);
                while ($pos !== false) {
                    if ($pos) {
                        $string = substr($string, 0, $pos) . $replacement . substr($string, $pos + 1);
                    }
                    elseif ($pos == strlen($string) - 1) {
                        $string = substr($string, 0, $pos) . $replacement;
                    }
                    else {
                        $string = $replacement . substr($string, $pos + 1);
                    }
                    $pos = strpos($string, $symbol, $pos + 1);
                }
            }
            return $string;
        }

        function contentctrl($content, $type) {
            if (strpos("var|label", $type) !== false) {
                $content = substr($content, strpos($content, "@") + 1);
                if (preg_match(self::varnlabelpattern, $content)) {
                    fwrite(STDERR,  "ERROR: Forbidden character in var|label name!\n");
                    fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                    exit(23);
                }
                else { return; }
            }
            if (!strcmp("int", $type)) {
                if (preg_match("/[^0-9|^-]/", $content)) {
                    fwrite(STDERR,  "ERROR: Forbidden character in int value! Only integer numbers!\n");
                    fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                    exit(23);
                }
                else { return; }
            }
            if (!strcmp("bool", $type)) {
                if (preg_match("/[^true|^false]/", $content)) {
                    fwrite(STDERR,  "ERROR: Forbidden character in bool value! Only \"true\" or \"false\"!\n");
                    fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                    exit(23);
                }
                else { return; }
            }
            if (!strcmp("nil", $type)) {
                if (preg_match("/[^nil]/", $content)) {
                    fwrite(STDERR,  "ERROR: Forbidden character in nil value! Only \"nil\"!\n");
                    fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                    exit(23);
                }
                else { return; }
            }
            if (!strcmp("string", $type)) {
                while (($pos = strpos($content, "\\")) !== false) {
                    $content = substr($content, $pos + 1);
                    $number = substr($content, 0, 3);
                    if (!ctype_digit($number)) {
                        fwrite(STDERR,  "ERROR: Invalid escape sequence!\n");
                        fwrite(STDERR, "Error occured here: line " . $this -> line . ", argument " . $this -> arg_pos . ".\n");
                        exit(23);
                    }
                }
                return;
            }
        }

        function deftype($var) {
            if (str_starts_with($var, "GF@") or str_starts_with($var, "LF@") or str_starts_with($var, "TF@")) {
                return "var";
            }
            if (str_starts_with($var, "int@") or str_starts_with($var, "string@") or 
                str_starts_with($var, "bool@") or str_starts_with($var, "nil@")) {
                $pos = strpos($var, "@");
                return substr($var, 0, $pos);
            }
            if (!strcmp($var, "int") or !strcmp($var, "string") or !strcmp($var, "bool")) {
                return "type";
            }
            return "label";
        }

        function rmcoms($strvar) {
            if  ($strvar[0] ?? false and
                ($pos = strpos($strvar, "#") or $strvar[0] == '#')) {
                if ($this -> statistics["comments"][0]) {
                    $this -> statistics["comments"][1]++;
                }
                $strvar = substr($strvar, 0, $pos);
            }
            return $strvar;
        }

        function __destruct() {
            fclose($this -> input);
            fclose($this -> output);

            if ($this -> options) {
                foreach ($this -> jumps as $label => $line) {
                    if (($this -> labels[$label] ?? false) > $line and $this -> statistics["fwjumps"][0]) {
                        $this -> statistics["fwjumps"][1]++;
                    }
                    elseif (($this -> labels[$label] ?? true) < $line and $this -> statistics["backjumps"][0]) {
                        $this -> statistics["backjumps"][1]++;
                    }
                    elseif (($this -> labels[$label] ?? false) === false and $this -> statistics["badjumps"][0]) {
                        $this -> statistics["badjumps"][1]++;
                    }
                }

                foreach ($this -> options as $filename => $parameters_array) {
                    $statsout = fopen($filename, "w+") or exit(12);
                    foreach ($parameters_array as $parameter) {
                        fwrite($statsout, $parameter . " = " . $this -> statistics[$parameter][1] . "\n");
                    }
                    fclose($statsout);
                }
            }
            //echo "Destructed!<br>";
        }
    }
    
    function do_some_magic($in, $out) {
        //read arguments from terminal
        $options = read_args();

        //in ver 1.0 it opened a given file and created an .out file
        $process = new IOStream($in, $out, $options);
        
        //There MUST be a ".IPPcode22" head line in the file, otherwise error
        $process -> headctrl();
        
        //open XML program block
        fwrite($process -> output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<program language=\"IPPcode22\">\n");
        
        //Parsing starts after finding head line
        while ($process -> opproc()) {
        }
        
        //close XML program block
        fwrite($process -> output, "</program>\n");
    }
    //BONUS!!!
    function read_args() {
        $shortopts = "";
        $longopts = array(
            "help", "stats:", "loc", "comments", "labels", "jumps", "fwjumps", "backjumps", "badjumps"
        );
        $options = getopt($shortopts, $longopts);
        if ($options) {
            if (array_key_exists("help", $options) and count($options) == 1) {
                fwrite(STDOUT, MANUAL);
                exit(0);
            }
            elseif (array_key_exists("help", $options) and count($options) < 1) {
                fwrite(STDERR, "ERROR: cannot combine \"--help\" parameter with others!\n");
                exit(10);
            }

            //if no "help" but there still were args -> build an associative array
            //every filename is a key, any other parameter goes to value array
            $chunked_opts = array();
            $value = array();
            $key = "";
            $max = 1;
            for ($i = 0; $i < $max; $i++) {
                foreach ($options as $parameter => $associative) {
                    //find the longest array in options array
                    if (is_array($associative)) {
                        if ($max < count($associative)) {
                            $max = count($associative);
                        }
                    }
                    //if key wasn't set yet, get one
                    if (!strcmp($parameter, "stats") and !$key) {
                        //gotta check, if it's an array of values
                        if (is_array($associative)) {
                            $key = $associative[$i] ?? false;
                            if (!$key) {
                                fwrite(STDERR, "ERROR: parameter \"--stats=\"*filename*\"\" required in order to continue!\n");
                                exit(10);
                            }
                        }
                        //if it isn't an array and i > 0 -> error
                        elseif (!is_array($associative) and $i > 0) {
                            fwrite(STDERR, "ERROR: parameter \"--stats=\"*filename*\"\" required in order to continue!\n");
                            exit(10);
                        }
                        else {
                            $key = $associative;
                        }
                        continue;
                    }
                    //found another key
                    elseif (!strcmp($parameter, "stats") and $key) {
                        //if it already exists -> error. Can't have >1 same keys!
                        if ($chunked_opts[$parameter] ?? false) {
                            fwrite(STDERR, "ERROR: this file is already in use!\n");
                            exit(12);
                        }
                        $chunked_opts += array($key => $value);
                        $value = array();
                        //same check conditions
                        if (is_array($associative)) {
                            $key = $associative[$i] ?? false;
                            if (!$key) {
                                fwrite(STDERR, "ERROR: parameter \"--stats=\"*filename*\"\" required in order to continue!\n");
                                exit(10);
                            }
                        }
                        elseif (!is_array($associative) and $i > 0) {
                            fwrite(STDERR, "ERROR: parameter \"--stats=\"*filename*\"\" required in order to continue!\n");
                            exit(10);
                        }
                        continue;
                    }
                    elseif (strcmp($parameter, "stats") and $key) {
                        if (is_array($associative)) {
                            $write = $associative[$i] ?? true;
                            if (!$write) {
                                $value[] = $parameter;
                            }
                        }
                        elseif (!is_array($associative) and $i == 0) {
                            $value[] = $parameter;
                        }
                        continue;
                    }
                    fwrite(STDERR, "ERROR: parameter \"--stats=\"*filename*\"\" required in order to continue!\n");
                    exit(10);
                }
            }
            $chunked_opts += array($key => $value);
            return $chunked_opts;
            //What next, genius?
        }
        else if ($options === false) {
            fwrite(STDERR, "ERROR: internal error!\n");
            exit(99);
        }
    }

    ini_set('display_errors', 'stderr');
    
    //do parse
    do_some_magic("php://stdin", "php://stdout");

    exit(0);
?>
