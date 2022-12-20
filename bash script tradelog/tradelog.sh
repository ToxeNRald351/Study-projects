#! /bin/bash

function after(){
    awk -v value="${time}" -F";" '{
        do {
            if ($1 > value)
                print $0
        } while (getline)
    }'
}

function before(){
    awk -v value="${time}" -F";" '{
        do {
            if ($1 < value)
                print $0
        } while (getline)
    }' 
}

function list-tick(){
    awk -F";" '{
        list = $2
        while (getline)
            list = list"\n"$2
        print list
    }' | sort -u
}

function profit(){
    awk -F";" '{
        ibuy = 0
        isell = 0
        do {
            if ($3 ~ /buy/)
                ibuy += $4 * $6
            if ($3 ~ /sell/)
                isell += $4 * $6
        } while (getline)
        profit = isell - ibuy
        printf "%4.2f", profit
    }' 
}

function pos(){
    sort -t ';' -k 2,2 | awk -F";" '{
        ticker = $2
        value = 0
        number = 0
        do {
            if ($2 != ticker)
            {
                printf "%-10s: %4.2f\n", ticker, value
                ticker = $2
                number = 0
            }
            if ($3 ~ /buy/)
                number += $6
            if ($3 ~ /sell/)
                number -= $6
            value = $4 * number
        } while (getline)
        value = $4 * number
        printf "%-10s: %4.2f\n", ticker, value
    }' | sort -n -k 3 -r | awk -F ":" '{
        a = length($2)
        print $0
        while (getline)
            print $0
        b = length($2)
        if (a > b)
            print a
        else
            print b
    }' | sort -n -k 1,1 -r | awk -F ":" '{
        a = $0
        while (getline)
        {
            printf "%-10s:", $1
            for (i = length($2); i < a; i++)
                printf " "
            printf " %.2f\n", $2
        }
    }' | sort -n -k 3 -r
}

function last-price(){
    sort -t ';' -k 2,2 | awk -F";" '{
        ticker = $2
        value = 0
        do {
            if ($2 != ticker)
            {
                printf "%-10s: %4.2f\n", ticker, value
                ticker = $2
            }
            value = $4
        } while (getline)
        printf "%-10s: %4.2f\n", ticker, value
    }' | sort -n -k 3 -r | awk -F ":" '{
        a = length($2)
        print $0
        while (getline)
            print $0
        b = length($2)
        if (a > b)
            print a
        else
            print b
    }' | sort -n -k 1,1 -r | awk -F ":" '{
        a = $0
        while (getline)
        {
            printf "%-10s:", $1
            for (i = length($2); i < a; i++)
                printf " "
            printf " %.2f\n", $2
        }
    }' | sort -n -k 1,1
}

function hist-ord(){
    sort -t ';' -k 2,2 | awk -v width="${hist_width}" -F";" '{
        ticker = $2
        value = 0
        hist_len = ""
        do {
            if ($2 != ticker) 
            {
                value = int(value/width)
                for (i = 0; i < value; i++)
                    hist_len = hist_len "#"
                printf "%-10s: %s\n", ticker, hist_len
                value = 0
                ticker = $2
                hist_len = ""
            }
            value++
        } while (getline)
        if (value > 0)
        {
            value = int(value/width)
            for (i = 0; i < value; i++)
                hist_len = hist_len "#"
            printf "%-10s: %s\n", ticker, hist_len
        }
    }' 
}

function graph-pos(){
    sort -t ';' -k 2,2 | awk -F";" '{
        ticker = $2
        value = 0
        number = 0
        do {
            if ($2 != ticker)
            {
                printf "%-10s: %4.2f\n", ticker, value
                ticker = $2
                number = 0
            }
            if ($3 ~ /buy/)
                number += $6
            if ($3 ~ /sell/)
                number -= $6
            value = $4 * number
        } while (getline)
        value = $4 * number
        printf "%-10s: %4.2f\n", ticker, value
    }' | sort -k 3 -n -r | tr -d [:blank:] | awk -v width="${graph_width}" -F":" '{
        do {
            graph_len = int($2 / width)
            printf "%-10s:", $1
            if (graph_len > 0)
            {
                printf " "
                for (i = 0; i < graph_len; i++)
                    printf "#"
                printf "\n"
            }
            if (graph_len < 0)
            {
                printf " "
                for (i = 0; i > graph_len; i--)
                    printf "!"
                printf "\n"
            }
            if (graph_len == 0)
                printf "\n"
        } while (getline)
    }' | sort -t ';' -k 2,2
}

hist_width=1
graph_width=1000
temp=""
for ((i=0; i <= $#; i++))
do
    if [[ "${!i}" =~ .*".log.gz" ]]
    then
        temp="$temp$(zcat ${!i})"
    elif [[ "${!i}" =~ .*".log" ]]
    then
        temp="$temp$(cat ${!i})"
    fi
done
if [[ $temp == "" ]]
then
    temp="$(awk '{print $0}')"
fi
while [ -n "$1" ]
do
    case "$1" in
        -a) 
            time="$2"
            if [ ${#time} != 19 ]
            then break
            fi
            temp="$(echo "$temp" | after $time)"
            shift 2 ;;
        -b) 
            time="$2"
            if [ ${#time} != 19 ]
            then break
            fi
            temp="$(echo "$temp" | before $time)"
            shift 2 ;;
        -t) 
            shift
            tickers="$1;"
            while [[ $2 == "-t" ]]
            do
                shift 2
                tickers+="|$1;"
                if [[ -z $2 ]]
                then break
                fi
            done
            temp="$(echo "$temp" | egrep "$tickers")" ;;
        -w) 
            shift
            let "hist_width = $1 * $hist_width"
            let "graph_width = $1 * $graph_width" ;;
        -h|--help) 
            echo " " | awk '{
                print "POUŽITÍ: tradelog [-h|--help] [FILTR] [PŘÍKAZ] [LOG [LOG2 [...]]\n\
VOLBY:\n\
    PŘÍKAZ může být jeden z:\n\
        list-tick – výpis seznamu vyskytujících se burzovních symbolů, tzv. “tickerů”.\n\
        profit – výpis celkového zisku z uzavřených pozic.\n\
        pos – výpis hodnot aktuálně držených pozic seřazených sestupně dle hodnoty.\n\
        last-price – výpis poslední známé ceny pro každý ticker.\n\
        hist-ord – výpis histogramu počtu transakcí dle tickeru.\n\
        graph-pos – výpis grafu hodnot držených pozic dle tickeru.\n\
    FILTR může být kombinace následujících:\n\
        -a `DATETIME` – after: jsou uvažovány pouze záznamy PO tomto datu (bez tohoto data). DATETIME je formátu YYYY-MM-DD HH:MM:SS. Musí byt uzavřen v jednoduchých uvozovkách\n\
        -b `DATETIME` – before: jsou uvažovány pouze záznamy PŘED tímto datem (bez tohoto data).\n\
        -t TICKER – jsou uvažovány pouze záznamy odpovídající danému tickeru. Při více výskytech přepínače se bere množina všech uvedených tickerů.\n\
        -w WIDTH – u výpisu grafů nastavuje jejich šířku, tedy délku nejdelšího řádku na WIDTH. Tedy, WIDTH musí být kladné celé číslo. Více výskytů přepínače je chybné spuštění."
                exit
            }'
            exit;;
        *)
            if [ $1 == 'list-tick' ]
            then
                shift
                temp="$(echo "$temp" | list-tick)"
            elif [ $1 == 'profit' ]
            then
                shift
                temp="$(echo "$temp" | profit)"
            elif [ $1 == 'pos' ]
            then
                shift
                temp="$(echo "$temp" | pos)"
            elif [ $1 == 'last-price' ]
            then
                shift
                temp="$(echo "$temp" | last-price)"
            elif [ $1 == 'hist-ord' ]
            then
                shift
                temp="$(echo "$temp" | hist-ord $hist_width)"
            elif [ $1 == 'graph-pos' ]
            then
                shift
                temp="$(echo "$temp" | graph-pos $graph_width)"
            fi;;
    esac
    shift
done
echo "$temp"
exit
