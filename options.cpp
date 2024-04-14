#include <limits>
#include "options.h"
#include "win32util.h"
#include <getopt.h>
#include "metadata.h"

bool is_seekable(int fd) {
    // Attempt to seek to the current position to check if the file descriptor is seekable
    return lseek(fd, 0, SEEK_CUR) != -1;
}

static option long_options[] = {
#ifdef QAAC
    { "formats", no_argument, 0, 'fmts' },
    { "abr", required_argument, 0, 'a' },
    { "tvbr", required_argument, 0, 'V' },
    { "cvbr", required_argument, 0, 'v' },
    { "cbr", required_argument, 0, 'c' },
    { "he", no_argument, 0, 'aach' },
    { "quality", required_argument, 0, 'q' },
    { "adts", no_argument, 0, 'ADTS' },
    { "no-smart-padding", no_argument, 0, 'nspd' },
    { "native-resampler", optional_argument, 0, 'nsrc' },
#endif
#ifdef REFALAC
    { "fast", no_argument, 0, 'afst' },
#endif
    { "check", no_argument, 0, 'chck' },
    { "alac", no_argument, 0, 'A' },
    { "decode", no_argument, 0, 'D' },
    { "play", no_argument, 0, 'play' },
    { "caf", no_argument, 0, 'caff' },
    { "no-optimize", no_argument, 0, 'noop' },
    { "bits-per-sample", required_argument, 0, 'b' },
    { "no-dither", no_argument, 0, 'ndit' },
    { "rate", required_argument, 0, 'r' },
    { "lowpass", required_argument, 0, 'lpf ' },
    { "peak", no_argument, 0, 'peak' },
    { "normalize", no_argument, 0, 'N' },
    { "gain", required_argument, 0, 'gain' },
    { "drc", required_argument, 0, 'drc ' },
    { "limiter", no_argument, 0, 'limt' },
    { "start", required_argument, 0, 'from' },
    { "end", required_argument, 0, 'end ' },
    { "delay", required_argument, 0, 'dlay' },
    { "no-delay", no_argument, 0, 'ndly' },
    { "num-priming", required_argument, 0, 'encd' },
    { "gapless-mode", required_argument, 0, 'gapm' },
    { "matrix-preset", required_argument, 0, 'mixp' },
    { "matrix-file", required_argument, 0, 'mixm' },
    { "no-matrix-normalize", no_argument, 0, 'nmxn' },
    { "chanmap", required_argument, 0, 'cmap' },
    { "chanmask", required_argument, 0, 'mask' },
    { "help", no_argument, 0, 'h' },
    { "silent", no_argument, 0, 's' },
    { "verbose", no_argument, 0, 'verb' },
    { "stat", no_argument, 0, 'S' },
    { "threading", no_argument, 0, 'thrd' },
    { "nice", no_argument, 0, 'n' },
    { "sort-args", no_argument, 0, 'soar' },
    { "tmpdir", required_argument, 0, 'tmpd' },
    { "text-codepage", required_argument, 0, 'txcp' },
    { "raw", no_argument, 0, 'R' },
    { "raw-channels", required_argument, 0,  'Rchn' },
    { "raw-rate", required_argument, 0,  'Rrat' },
    { "raw-format", required_argument, 0,  'Rfmt' },
    { "ignorelength", no_argument, 0, 'i' },
    { "concat", no_argument, 0, 'cat ' },
    { "cue-tracks", required_argument, 0, 'ctrk' },
    { "fname-from-tag", no_argument, 0, 'fftg' },
    { "fname-format", required_argument, 0, 'nfmt' },
    { "log", required_argument, 0, 'log ' },
    { "title", required_argument, 0, (int)Tag::kTitle },
    { "artist", required_argument, 0, (int)Tag::kArtist },
    { "band", required_argument, 0, (int)Tag::kAlbumArtist },
    { "album", required_argument, 0, (int)Tag::kAlbum },
    { "grouping", required_argument, 0, (int)Tag::kGrouping },
    { "composer", required_argument, 0, (int)Tag::kComposer },
    { "comment", required_argument, 0, (int)Tag::kComment },
    { "genre", required_argument, 0, (int)Tag::kGenre },
    { "date", required_argument, 0, (int)Tag::kDate },
    { "track", required_argument, 0, (int)Tag::kTrack },
    { "disk", required_argument, 0, (int)Tag::kDisk },
    { "compilation", optional_argument, 0, (int)Tag::kCompilation },
    { "lyrics", required_argument, 0, (int)Tag::kLyrics },
    { "artwork", required_argument, 0, (int)Tag::kArtwork },
    { "artwork-size", required_argument, 0, 'atsz' },
    { "copy-artwork", no_argument, 0, 'cpat' },
    { "chapter", required_argument, 0, 'chap' },
    { "tag", required_argument, 0, 'tag ' },
    { "long-tag", required_argument, 0, 'ltag' },
    { "tag-from-file", required_argument, 0, 'tagf' },
    { 0, 0, 0, 0 }
};
static const uint32_t tag_keys[] = {
    Tag::kTitle,
    Tag::kArtist,
    Tag::kAlbumArtist,
    Tag::kAlbum,
    Tag::kGrouping,
    Tag::kComposer,
    Tag::kComment,
    Tag::kGenre,
    Tag::kDate,
    Tag::kTrack,
    Tag::kDisk,
    Tag::kCompilation,
    Tag::kArtwork,
    Tag::kLyrics
};
const uint32_t * const tag_keys_end = tag_keys + util::sizeof_array(tag_keys);

const char *get_qaac_version();

#ifdef REFALAC
#define PROGNAME "refalac"
#elif defined QAAC
#define PROGNAME "qaac"
#endif

static
void usage()
{
    std::printf("%s %s\n%s", PROGNAME, get_qaac_version(),
"Usage: " PROGNAME " [options] infiles....\n"
"\n"
"\"-\" as infile means stdin.\n"
#ifndef REFALAC
"On ADTS/WAV output mode, \"-\" as outfile means stdout.\n"
#endif
"\n"
"Main options:\n"
#ifdef QAAC
"--formats              Show available AAC formats and exit\n"
"-a, --abr <bitrate>    AAC ABR mode / bitrate\n"
"-V, --tvbr <n>         AAC True VBR mode / quality [0-127]\n"
"-v, --cvbr <bitrate>   AAC Constrained VBR mode / bitrate\n"
"-c, --cbr <bitrate>    AAC CBR mode / bitrate\n"
"                       For -a, -v, -c, \"0\" as bitrate means \"highest\".\n"
"                       Highest bitrate available is automatically chosen.\n"
"                       For LC, default is -V90\n"
"                       For HE, default is -v0\n"
"--he                   HE AAC mode (TVBR is not available)\n"
"-q, --quality <n>      AAC encoding Quality [0-2]\n"
"--adts                 ADTS output (AAC only)\n"
"--no-smart-padding     Don't apply smart padding for gapless playback.\n"
"                       By default, beginning and ending of input is\n"
"                       extrapolated to achieve smooth transition between\n"
"                       songs. This option also works as a workaround for\n"
"                       bug of CoreAudio HE-AAC encoder that stops encoding\n"
"                       1 frame too early.\n"
"                       Setting this option can lead to gapless playback\n"
"                       issue especially on HE-AAC.\n"
"                       However, resulting bitstream will be identical with\n"
"                       iTunes only when this option is set.\n"
#endif
#ifdef REFALAC
"--fast                 Fast stereo encoding mode.\n"
#endif
"-d <dirname>           Output directory. Default is current working dir.\n"
"--check                Show library versions and exit.\n"
"-A, --alac             ALAC encoding mode\n"
"-D, --decode           Decode to a WAV file.\n"
"--caf                  Output to CAF file instead of M4A/WAV/AAC.\n"
"--play                 Decode to a WaveOut device (playback).\n"
"-r, --rate <keep|auto|n>\n"
"                       keep: output sampling rate will be same as input\n"
"                             if possible.\n"
"                       auto: output sampling rate will be automatically\n"
"                             chosen by encoder.\n"
"                       n: desired output sampling rate in Hz.\n"
"--lowpass <number>     Specify lowpass filter cut-off frequency in Hz.\n"
"                       Use this when you want lower cut-off than\n"
"                       Apple default.\n"
"-b, --bits-per-sample <n>\n"
"                       Bits per sample of output (for WAV/ALAC only)\n"
"--no-dither            Turn off dither when quantizing to lower bit depth.\n" 
"--peak                 Scan + print peak (don't generate output file).\n"
"                       Cannot be used with encoding mode or -D.\n"
"                       When DSP options are set, peak is computed \n"
"                       after all DSP filters have been applied.\n"
"--gain <f>             Adjust gain by f dB.\n"
"                       Use negative value to decrese gain, when you want to\n"
"                       avoid clipping introduced by DSP.\n"
"-N, --normalize        Normalize (works in two pass. can generate HUGE\n"
"                       tempfile for large piped input)\n"
"--drc <thresh:ratio:knee:attack:release>\n"
"                       Dynamic range compression.\n"
"                       Loud parts over threshold are attenuated by ratio.\n"
"                         thresh:  threshold (in dBFS, < 0.0)\n"
"                         ratio:   compression ratio (> 1.0)\n"
"                         knee:    knee width (in dB, >= 0.0)\n"
"                         attack:  attack time (in millis, >= 0.0)\n"
"                         release: release time (in millis, >= 0.0)\n"
"--limiter              Apply smart limiter that softly clips portions\n"
"                       where peak exceeds (near) 0dBFS\n"
"--start <[[hh:]mm:]ss[.ss..]|<n>s|<mm:ss:ff>f>\n"
"                       Specify start point of the input.\n"
"                       You specify either in seconds(hh:mm:ss.sss..form) or\n"
"                       number of samples followed by 's' or\n"
"                       cuesheet frames(mm:ss:ff form) followed by 'f'.\n"
"                       When negative value is given, instead of trimming,\n"
"                       specified amount of silence is prepended.\n"
"                       Example:\n"
"                         --start 4010160s : start at 4010160 samples\n"
"                         --start 1:30:70f : same as above, in cuepoint\n"
"                         --start 1:30.93333 : same as above\n"
"--end <[[hh:]mm:]ss[.ss..]|<n>s|<mm:ss:ff>f>\n"
"                       Specify end point of the input (exclusive).\n"
"--delay <[[hh:]mm:]ss[.ss..]|<n>s|<mm:ss:ff>f>\n"
"                       Same as --start, with the sign reversed.\n"
"                       Positive value will prepend silence.\n"
"                       (This option exists due to historical reason)\n"
"--no-delay             Compensate encoder delay by prepending 960 samples \n"
"                       of silence, then trimming 3 AAC frames from \n"
"                       the beginning (and also tweak iTunSMPB).\n"
"                       This option is mainly intended for resolving\n"
"                       A/V sync issue of video. \n"
"--num-priming <n>      (Experimental). Set arbitrary number of priming\n"
"                       samples in range from 0 to 2112 (default 2112).\n"
"                       Applicable only for AAC LC.\n"
"                       --num-priming=0 is the same as --no-delay.\n"
"                       Doesn't work with --no-smart-padding.\n"
"--gapless-mode <n>     Encoder delay signaling for gapless playback.\n"
"                         0: iTunSMPB (default)\n"
"                         1: ISO standard (elst + sbgp + sgpd)\n"
"                         2: Both\n"
"--matrix-preset <name> Specify user defined preset for matrix mixer.\n"
"--matrix-file <file>   Matrix file for remix.\n"
"--no-matrix-normalize  Don't automatically normalize(scale) matrix\n"
"                       coefficients for the matrix mixer.\n"
"--chanmap <n1,n2...>   Rearrange input channels to the specified order.\n"
"                       Example:\n"
"                         --chanmap 2,1 -> swap L and R.\n"
"                         --chanmap 2,3,1 -> C+L+R -> L+R+C.\n"
"--chanmask <n>         Force input channel mask(bitmap).\n"
"                       Either decimal or hex number with 0x prefix\n"
"                       can be used.\n"
"                       When 0 is given, qaac works as if no channel mask is\n"
"                       present in the source and picks default layout.\n"
"--no-optimize          Don't optimize MP4 container after encoding.\n"
"--tmpdir <dirname>     Specify temporary directory. Default is %TMP%\n"
"-s, --silent           Suppress console messages.\n"
"--verbose              More verbose console messages.\n"
"-i, --ignorelength     Assume WAV input and ignore the data chunk length.\n"
"--threading            Enable multi-threading.\n"
"-n, --nice             Give lower process priority.\n"
"--sort-args            Sort filenames given by command line arguments.\n"
"--text-codepage <n>    Specify text code page of cuesheet/chapter/lyrics.\n"
"                       Example: 1252 for Latin-1, 65001 for UTF-8.\n"
"                       Use this when bogus values are written into tags\n"
"                       due to automatic encoding detection failure.\n"
"-S, --stat             Save bitrate statistics into file.\n"
"--log <filename>       Output message to file.\n"
"\n"
"Option for output filename generation:\n"
"--fname-from-tag       Generate filename based on metadata of input.\n"
"                       By default, output filename will be the same as input\n"
"                       (only different by the file extension).\n"
"                       Name generation can be tweaked by --fname-format.\n"
"--fname-format <string>   Format string for output filename.\n"
"\n"
"Option for single output:\n"
"-o <filename>          Specify output filename\n"
"--concat               Encodes whole inputs into a single file. \n"
"                       Requires output filename (with -o)\n"
"\n"
"Option for cuesheet input only:\n"
"--cue-tracks <n[-n][,n[-n]]*>\n"
"                       Limit extraction to specified tracks.\n"
"                       Tracks can be specified with comma separated numbers.\n"
"                       Hyphen can be used to denote range of numbers.\n"
"                       Tracks non-existent in the cue are just ignored.\n"
"                       Numbers must be in the range 0-99.\n"
"                       Example:\n"
"                         --cue-tracks 1-3,6-9,11\n"
"                           -> equivalent to --cue-tracks 1,2,3,6,7,8,9,11\n"
"                         --cue-tracks 2-99\n"
"                           -> can be used to skip first track (and HTOA)\n"
"\n"
"Options for Raw PCM input only:\n"
"-R, --raw              Raw PCM input.\n"
"--raw-channels <n>     Number of channels, default 2.\n"
"--raw-rate     <n>     Sample rate, default 44100.\n"
"--raw-format   <str>   Sample format, default S16L.\n"
"                       Sample format spec:\n"
"                       1st char: S(igned) | U(nsigned) | F(loat)\n"
"                       2nd part: Bitwidth\n"
"                       Last part: L(ittle Endian) | B(ig Endian)\n"
"                       Last part can be omitted, L is assumed by default.\n"
"                       Cases are ignored. u16b is OK.\n"
"\n"
#ifdef QAAC
"Options for CoreAudio sample rate converter:\n"
"--native-resampler[=line|norm|bats,n]\n"
"                       Arguments followed by '=' are optional.\n"
"                       First argument before comma is complexity from\n"
"                       one of the following:\n"
"                         line: linear (worst, don't use this)\n"
"                         norm: normal\n"
"                         bats: mastering (best, default)\n"
"                       Second argument after comma is integer quality\n"
"                       between 0-127 (default 0).\n"
"                       Example:\n"
"                         --native-resampler\n"
"                         --native-resampler=norm,96\n"
"\n"
#endif
"Tagging options:\n"
" (same value is set to all files, so use with care for multiple files)\n"
"--title <string>\n"
"--artist <string>\n"
"--band <string>       This means \"Album Artist\".\n"
"--album <string>\n"
"--grouping <string>\n"
"--composer <string>\n"
"--comment <string>\n"
"--genre <string>\n"
"--date <string>\n"
"--track <number[/total]>\n"
"--disk <number[/total]>\n"
"--compilation[=0|1]\n"
"                      By default, iTunes compilation flag is not set.\n"
"                      --compilation or --compilation=1 sets flag on.\n"
"                      --compilation=0 is same as default.\n"
"--lyrics <filename>\n"
"--artwork <filename>\n"
"--artwork-size <n>    Specify maximum width or height of artwork in pixels.\n"
"                      If specified artwork (with --artwork) is larger than\n"
"                      this, artwork is automatically resized.\n"
"--copy-artwork        Copy front cover art(APIC:type 3) from the source.\n"
"                      When --artwork is also given, this option is ignored.\n"
"--chapter <filename>\n"
"                      Set chapter from file.\n"
"--tag <fcc>:<value>\n"
"                      Set iTunes pre-defined tag with fourcc key\n"
"                      and value.\n"
"                      1) When key starts with U+00A9 (copyright sign),\n"
"                         you can use 3 chars starting from the second char\n"
"                         instead.\n"
"                      2) Some known tags having type other than UTF-8 string\n"
"                         are taken care of. Others are just stored as UTF-8\n"
"                         string.\n"
"--tag-from-file <fcc>:<path>\n"
"                      Same as above, but value is read from file.\n"
"--long-tag <key>:<value>\n"
"                      Set long tag (iTunes custom metadata) with \n"
"                      arbitrary key/value pair. Value is always stored as\n"
"                      UTF8 string.\n"
    );
}

static void complain(const char *s)
{
    std::fputs(s, stderr);
}

#ifdef QAAC
static const char * const short_opts = "hDo:d:b:r:insRSNAa:V:v:c:q:";
#endif
#ifdef REFALAC
static const char * const short_opts = "hDo:d:b:r:insRSNA";
#endif

bool Options::parse(int &argc, char **&argv)
{
    int ch, pos;
    while ((ch = getopt_long(argc, argv,
                                   short_opts, long_options, 0)) != EOF)
    {
        if (ch == 'h')
            return usage(), false;
        else if (ch == 'chck')
            this->check_only = true;
        else if (ch == 'fmts')
            this->print_available_formats = true;
        else if (ch == 'o')
            this->ofilename = optarg;
        else if (ch == 'd')
            this->outdir = optarg;
        else if (ch < 0xff && (pos = strutil::strindex("cavV", ch)) >= 0) {
            if ((this->output_format && !isAAC()) || this->method != -1) {
                complain("Encoding mode options are exclusive.\n");
                return false;
            }
            this->method = pos;
            if (std::sscanf(optarg, "%lf", &this->bitrate) != 1) {
                complain("AAC Bitrate/Quality must be an integer.\n");
                return false;
            }
        }
        else if (ch == 'A') {
            if ((this->output_format && !isALAC()) || this->method != -1) {
                complain("Encoding mode options are exclusive.\n");
                return false;
            }
            this->output_format = 'alac';
        }
        else if (ch == 'D') {
            if (this->output_format && !isLPCM()) {
                complain("Encoding mode options are exclusive.\n");
                return false;
            }
            this->output_format = 'lpcm';
        }
        else if (ch == 'aach') {
            if (this->output_format && !isAAC()) {
                complain("--he is only available for AAC.\n");
                return false;
            }
            this->output_format = 'aach';
        }
        else if (ch == 'play') {
            if (this->output_format && !isWaveOut()) {
                complain("--play cannot be specified with encoding mode.\n");
                return false;
            }
            this->output_format = 'play';
        }
        else if (ch == 'peak') {
            if (this->output_format && !isPeak()) {
                complain("--peak cannot be specified with encoding mode.\n");
                return false;
            }
            this->output_format = 'peak';
        }
        else if (ch == 'caff')
            this->is_caf = true;
        else if (ch == 'q') {
            if (std::sscanf(optarg, "%u", &this->quality) != 1) {
                complain("-q requires an integer.\n");
                return false;
            }
        }
        else if (ch == 'log ')
            this->logfilename = optarg;
        else if (ch == 'nspd')
            this->no_smart_padding = true;
        else if (ch == 'nsrc') {
            this->native_resampler = true;
            if (optarg) {
                strutil::Tokenizer<char> tokens(optarg, ",");
                char *tok;
                while ((tok = tokens.next())) {
                    int n;
                    if (std::sscanf(tok, "%u", &n) == 1)
                        this->native_resampler_quality = n;
                    else if (std::strlen(tok) == 4)
                        this->native_resampler_complexity =
                            util::fourcc(tok);
                    else {
                        complain("Invalid arg for --native-resampler.\n");
                        return false;
                    }
                }
            }
        }
        else if (ch == 'N')
            this->normalize = true;
        else if (ch == 's')
            this->verbose = 0;
        else if (ch == 'verb')
            this->verbose = 2;
        else if (ch == 'S')
            this->save_stat = true;
        else if (ch == 'n')
            this->nice = true;
        else if (ch == 'thrd')
            this->threading = true;
        else if (ch == 'i')
            this->ignore_length = true;
        else if (ch == 'R')
            this->is_raw = true;
        else if (ch == 'ADTS')
            this->is_adts = true;
        else if (ch == 'noop')
            this->no_optimize = true;
        else if (ch == 'cat ')
            this->concat = true;
        else if (ch == 'nfmt')
            this->fname_format = optarg;
        else if (ch == 'tmpd')
            this->tmpdir = optarg;
        else if (ch == 'nmxn')
            this->no_matrix_normalize = true;
        else if (ch == 'cmap') {
            strutil::Tokenizer<char> tokens(optarg, ",");
            char *tok;
            while ((tok = tokens.next()) != 0) {
                unsigned n;
                if (std::sscanf(tok, "%u", &n) == 1)
                    this->chanmap.push_back(n);
                else {
                    complain("Invalid arg for --chanmap.\n");
                    return false;
                }
            }
            uint32_t low = std::numeric_limits<int>::max();
            uint32_t high = 0;
            for (size_t i = 0; i < this->chanmap.size(); ++i) {
                uint32_t n = this->chanmap[i];
                if (n < low) low = n;
                if (n > high) high = n;
            }
            if (low < 1 || high > this->chanmap.size()) {
                complain("Invalid arg for --chanmap.\n");
                return false;
            }
        }
        else if (ch == 'r') {
            if (!std::strcmp(optarg, "keep"))
                this->rate = -1;
            else if (!std::strcmp(optarg, "auto"))
                this->rate = 0;
            else if (std::sscanf(optarg, "%u", &this->rate) != 1) {
                complain("Invalid arg for --rate.\n");
                return false;
            }
        }
        else if (ch == 'lpf ') {
            if (std::sscanf(optarg, "%u", &this->lowpass) != 1) {
                complain("--lowpass requires an integer.\n");
                return false;
            }
        }
        else if (ch == 'b') {
            uint32_t n;
            if (std::sscanf(optarg, "%u", &n) != 1) {
                complain("-b requires an integer.\n");
                return false;
            }
            if (n <= 1 || n > 32) {
                complain("Bits per sample is too low or too high.\n");
                return false;
            }
            this->bits_per_sample = n;
        }
        else if (ch == 'ndit') {
            this->no_dither = true;
        }
        else if (ch == 'mask') {
            if (std::sscanf(optarg, "%i", &this->chanmask) != 1) {
                complain("--chanmask requires an integer.\n");
                return false;
            }
        }
        else if (ch == 'Rchn') {
            if (std::sscanf(optarg, "%u", &this->raw_channels) != 1) {
                complain("--raw-channels requires an integer.\n");
                return false;
            }
            if (this->raw_channels == 0) {
                complain("Invalid --raw-channels value.\n");
                return false;
            }
            if (this->raw_channels > 8) {
                complain("--raw-channels too large.\n");
                return false;
            }
        }
        else if (ch == 'Rrat') {
            if (std::sscanf(optarg, "%u",
                             &this->raw_sample_rate) != 1) {
                complain("--raw-rate requires an integer.\n");
                return false;
            }
            if (this->raw_sample_rate == 0) {
                complain("Invalid --raw-rate value.\n");
                return false;
            }
        }
        else if (ch == 'Rfmt')
            this->raw_format = optarg;
        else if (ch == 'afst')
            this->alac_fast = true;
        else if (ch == 'gain') {
            if (std::sscanf(optarg, "%lf", &this->gain) != 1) {
                complain("--gain requires an floating point number.\n");
                return false;
            }
        }
        else if (ch == 'drc ') {
            double threshold, ratio, knee, attack, release;
            if (std::sscanf(optarg,
                             "%lf:%lf:%lf:%lf:%lf",
                             &threshold,
                             &ratio,
                             &knee,
                             &attack,
                             &release) != 5) {
                complain("--drc requires 5 parameters.\n");
                return false;
            }
            if (threshold >= 0.0) {
                complain("DRC threshold cannot be positive.\n");
                return false;
            }
            if (ratio <= 1.0) {
                complain("DRC ratio has to be greater than 1.0\n");
                return false;
            }
            if (knee < 0.0) {
                complain("DRC knee width cannot be negative.\n");
                return false;
            }
            if (attack < 0.0) {
                complain("DRC attack time cannot be negative.\n");
                return false;
            }
            if (release < 0.0) {
                complain("DRC release time cannot be negative.\n");
                return false;
            }
            const char *p = optarg;
            for (int i = 0; i < 5; ++i) {
                p = std::strchr(p, ':');
                if (p) ++p;
            }
            this->drc_params.push_back(DRCParams(threshold, ratio, knee,
                                                 attack, release, p));
        }
        else if (ch == 'limt')
            this->limiter = true;
        else if (ch == 'from')
            this->start = optarg;
        else if (ch == 'end ')
            this->end = optarg;
        else if (ch == 'dlay')
            this->delay = optarg;
        else if (ch == 'ndly')
            this->num_priming = 0;
        else if (ch == 'encd') {
            if (std::sscanf(optarg, "%u", &this->num_priming) != 1) {
                complain("Invalid arg for --num-priming.\n");
                return false;
            }
            if (this->num_priming > 2112) {
                complain("num-priming must not be greater than 2112.\n");
                return false;
            }
        }
        else if (ch == 'soar')
            this->sort_args = true;
        else if (ch == 'gapm') {
            if (std::sscanf(optarg, "%u", &this->gapless_mode) != 1) {
                complain("Invalid arg for --gapless-mode.\n");
                return false;
            }
        }
        else if (ch == 'txcp') {
            if (std::sscanf(optarg, "%u", &this->textcp) != 1) {
                complain("--text-codepage requires code page number.\n");
                return false;
            }
        }
        else if (ch == 'ctrk') {
            if (!strutil::parse_numeric_ranges(optarg,
                                               &this->cue_tracks)) {
                complain("Invalid arg for --cue-tracks.\n");
                return false;
            }
        }
        else if (ch == 'atsz') {
            if (std::sscanf(optarg, "%u", &this->artwork_size) != 1) {
                complain("--artwork-size requires an integer.\n");
                return false;
            }
        }
        else if (ch == Tag::kArtwork)
            this->artwork_files.push_back(optarg);
        else if (ch == 'cpat')
            this->copy_artwork = true;
        else if (std::find(tag_keys, tag_keys_end, uint32_t(ch)) != tag_keys_end) {
            if (ch == Tag::kLyrics)
                this->ftagopts[ch] = optarg;
            else if (ch != Tag::kCompilation)
                this->tagopts[ch] = (optarg);
            else if (!optarg)
                this->tagopts[ch] = "1";
            else {
                int n;
                if (std::sscanf(optarg, "%d", &n) != 1) {
                    complain("Invalid --compilation option arg.\n");
                    return false;
                }
                this->tagopts[ch] = (optarg);
            }
        }
        else if (ch == 'tag ' || ch == 'tagf') {
            strutil::Tokenizer<char> tokens(optarg, ":");
            char *key = tokens.next();
            char *value = tokens.rest();
            size_t keylen = std::strlen(key);
            if (!value || (keylen != 3 && keylen != 4)) {
                complain("Invalid --tag option arg.\n");
                return false;
            }
            uint32_t fcc = (keylen == 3) ? 0xa9 : 0;
            char wc;
            while ((wc = *key++) != 0) {
                if (wc != 0xa9 && (wc < 0x20 || wc > 0x7e)) {
                    complain("Invalid fourcc for --tag.\n");
                    return false;
                }
                fcc = ((fcc << 8) | wc);
            }
            if (fcc == Tag::kArtwork)
                this->artwork_files.push_back(value);
            else if (ch == 'tag ')
                this->tagopts[fcc] = (value);
            else
                this->ftagopts[fcc] = value;
        }
        else if (ch == 'ltag') {
            strutil::Tokenizer<char> tokens(optarg, ":");
            char *key = tokens.next();
            char *value = tokens.rest();
            if (!value) {
                complain("Invalid arg for --long-tag.\n");
                return false;
            }
            this->longtags[(key)] = (value);
        }
        else if (ch == 'chap')
            this->chapter_file = optarg;
        else if (ch == 'mixp')
            this->remix_preset = optarg;
        else if (ch == 'mixm')
            this->remix_file = optarg;
        else if (ch == 'fftg')
            this->filename_from_tag = true;
        else
            return false;
    }
    argc -= optind;
    argv += optind;

    if (!argc && !this->check_only && !this->print_available_formats) {
        if (optind == 1)
            return usage(), false;
        else {
            complain("Input file name is required.\n");
            return false;
        }
    }
    if (argc > 1 && this->ofilename && !this->concat) {
        complain("-o is not available for multiple output.\n");
        return false;
    }
    if (!this->output_format) {
#ifdef REFALAC
        this->output_format = 'alac';
#else
        this->output_format = 'aac ';
#endif
    }
    if (isSBR() && this->method == kTVBR) {
        complain("TVBR is not available for HE.\n");
        return false;
    }
    if (isAAC() && this->method == -1) {
        this->method = isSBR() ? kCVBR : kTVBR;
        this->bitrate = isSBR() ? 0 : 90;
    }
    if (isMP4() && this->ofilename && !std::strcmp(this->ofilename, "-")) {
        if (!is_seekable(fileno(stdout))) {
            complain("MP4 piping is not supported.\n");
            return false;
        }
    }
    if (!isAAC() && this->is_adts) {
        complain("--adts is only available for AAC.\n");
        return false;
    }
    if (!isAAC() && this->quality != -1) {
        complain("-q is only available for AAC.\n");
        return false;
    }
    if (this->is_caf && this->is_adts) {
        complain("Can't use --caf and --adts at the same time.\n");
        return false;
    }
    if (this->ignore_length && this->is_raw) {
        complain("Can't use --ignorelength and --raw at the same time.\n");
        return false;
    }
    if (this->concat && argc > 1 && !this->ofilename &&
        !this->isPeak() && !this->isWaveOut()) {
        complain("--concat requires output filename (use -o option).\n");
        return false;
    }
    if ((!isAAC() || isSBR()) && this->num_priming != 2112) {
        complain("--num-priming is only applicable for AAC LC.\n");
        return false;
    }
    if (this->delay && this->start) {
        complain("Can't use --start and --delay at the same time.\n");
        return false;
    }
    if (this->quality == -1)
        this->quality = 2;
    return true;
}
