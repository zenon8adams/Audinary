const St             = imports.gi.St;
const Desklet        = imports.ui.desklet;
const DeskletManager = imports.ui.deskletManager;
const Settings       = imports.ui.settings;
const Extension      = imports.ui.extension;
const Lang           = imports.lang;
const Soup           = imports.gi.Soup;
const Clutter        = imports.gi.Clutter;
const GLib           = imports.gi.GLib;
const Config         = imports.misc.config;
const _httpSession   = new Soup.SessionAsync();
const byteArray      = imports.byteArray;
const Pango          = imports.gi.Pango;
const Gio            = imports.gi.Gio;
const Main           = imports.ui.main;

const HOME          = GLib.get_home_dir();
const BASE_URL      = "http://localhost:3800";
const DESKLET       = "DESKLET";
const UUID          = "audinary@zener-diode";
const DESKLET_DIR   = imports.ui.deskletManager.deskletMeta[ UUID].path;
const SETTINGS_FILE = DESKLET_DIR + "/settings-schema.json";

const DEFAULT_NOTIFICATION = "none";
const DEFAULT			   = "";

class TextualTimeDesklet extends Desklet.Desklet {
    constructor( metadata, deskletID) {
        super( metadata, deskletID);

        this._deskletID = deskletID;
        this._requestParams = {
            emitID:       true,
            fonts:        true,
            notification: DEFAULT_NOTIFICATION,
            track:        DEFAULT
        };

        this._features = {};

        this._textCasing = {
            mixed:      text => text,
            uppercase:  text => text.toUpperCase(),
            lowercase:  text => text.toLowerCase(),
            capitalize: text => text.length < 2 ? text : text[0].toUpperCase() + text.slice( 1).toLowerCase()
        };

        this._clockContainer = new St.BoxLayout( { vertical: true,
                                                   style_class: "clock_container_style"});
        this._timeText = new St.Label({ style_class: "time_label_style"});
        this._timeText.clutter_text.set_line_wrap( true);
        this._timeText.clutter_text.set_line_wrap_mode( Pango.WrapMode.WORD);
//        this._timeText.clutter_text.set_x_align( Clutter.ActorAlign.END);
        this._timeText.clutter_text.ellipsize = Pango.EllipsizeMode.NONE;
        this._timeText.clutter_text.set_x_expand( true);
        this._clockContainer.add( this._timeText);
        this.setContent( this._clockContainer);
        this.setHeader( _( "Textual Time"));

        this.settings = new Settings.DeskletSettings( this, UUID, deskletID);
        this.settings.bind( "notification",       "notification",      this._onSettingsChanged);
        this.settings.bind( "custom-track",       "audioTrack",        this._onSettingsChanged);
        this.settings.bind( "font",               "fontFamily",        this._onSettingsChanged);
        this.settings.bind( "font-bold",          "fontBold",          this._onSettingsChanged);
        this.settings.bind( "font-italic",        "fontItalic",        this._onSettingsChanged);
        this.settings.bind( "font-size",          "fontSize",          this._onSettingsChanged);
        this.settings.bind( "text-color",         "textColor",         this._onSettingsChanged);
        this.settings.bind( "text-opacity",       "textOpacity",       this._onSettingsChanged);
        this.settings.bind( "text-casing",        "textCasing",        this._onSettingsChanged);
        this.settings.bind( "background-color",   "backgroundColor",   this._onSettingsChanged);
        this.settings.bind( "background-opacity", "backgroundOpacity", this._onSettingsChanged);
        this.settings.bind( "locale-pack",        "localePack",        this._onSettingsChanged);

        this._timeText.set_text( "Loading...");

        global.log( "Initial setup completed!");
        this._interval = setInterval( this._queryTime.bind( this), 50);
    }

    _constructParams() {
        let params = "?";
        for( const key in this._requestParams)
        {
            let value = this._requestParams[ key];
            if( typeof value === "string")
                value = '"' + value + '"';
            params += `${ params.length > 1 ? "&" : ""}${key}=${ value}`;
        }

        return params;
    }

    _reloadFonts( filepath)
    {
        let [ok, contents] = GLib.file_get_contents( filepath);
        if( ok)
        {
            let map = JSON.parse( contents);
            map[ 'font'][ 'options'] = this._features.fonts;
            GLib.file_set_contents( filepath, JSON.stringify( map, null, 2));
        }
    }

    _queryTime() {
        let message = Soup.Message.new(  'GET', BASE_URL + this._constructParams());
        _httpSession.send_message( message);
        if ( message.status_code === Soup.KnownStatusCode.OK) {
            const response = JSON.parse( message.response_body.data.toString());
            if( response.fonts && response.fonts.length > 0)
            {
                this._requestParams.fonts = false;
                this._features.fonts = {};
                response.fonts.forEach( font => this._features.fonts[ font] = font);

                this._reloadFonts( SETTINGS_FILE);
                this._reloadFonts( `${HOME}/.${Config.PACKAGE_NAME}/configs/${UUID}/${UUID}.json`);
            }

			/*
			*  Check if there is a provided locale pack.
			*  If one exists and conforms to standard,
			*  set it as the new pack else,
			*  tell generator to generate with it's own imbued locale.
			*/

			if( this._requestParams.emitID)
			{
				if( response.i18n)
				{
					this._features.i18n = response.i18n.split( '$');
            		delete this._requestParams.i18n;
				}
				else if( !this._features.i18n)
				{
					this._requestParams.emitID = false;
					this._queryTime();
		    		return;
				}
			}

            let time = response.time;
            if( typeof time === 'number' && this._features.i18n)
                this._timeText.set_text( this._textCasing[ this.textCasing]( this._features.i18n[ time]));
            else
                this._timeText.set_text( this._textCasing[ this.textCasing]( time));
        }
        else
            this._timeText.set_text( this._textCasing[ this.textCasing]( "Loading..."));
    }

    _onSettingsChanged() {
        this._timeText.style = `
            font-family: ${ this.fontFamily};
            font-size: ${ this.fontSize}pt;
            color: ${ this._adjustedColor( this.textColor, this.textOpacity)};
            font-weight: ${ this.fontBold  ? "bold" : "normal"};
            font-style: ${ this.fontItalic ? "italic" : "normal"};
        `;

        this._clockContainer.style = `
            background-color: ${ this._adjustedColor( this.backgroundColor, this.backgroundOpacity)};
            padding: 20px 10px;
            border-radius: 10px;
            max-width: 220px;
        `;

        this._requestParams.notification = this.notification;
        this._requestParams.track        = this.audioTrack;

        if( this.localePack != "" && this._activeLocale != this.localePack)
        {
            this._requestParams.emitID = true;
            this._requestParams.i18n = this.localePack;
            this._activeLocale = this.localePack;
            delete this._features.i18n;
        }
    }

    _adjustedColor( applied, opacity)
    {
        const color = this._parseColor( applied);
        return `rgba(${color[ 0]},${color[ 1]},${color[ 2]}, ${opacity})`;
    }

    _extract( scheme, pos, values)
    {
        let weight = 0, at = pos, is_float = false, scale = 1;
        for( ; pos < scheme.length; ++pos)
        {
            const value = scheme[ pos];
            if( value == ' ')
                continue;

            if( value == '.')
            {
                is_float = true;
                continue;
            }

            if( value >= '0' && value <= '9')
            {
                weight = is_float ? scale * ( value - '0') + weight : 10 * weight + ( value - '0');
                scale *= .1;
            }
            else
                break;
        }

        if( at != pos)
            values.push( weight);

        return pos;
    }

    _parseColor( color)
    {
        const values = [];
        let pos = color.indexOf( '(') + 1;
        while( pos < color.length)
            pos = this._extract( color, pos, values) + 1;

        return values;
    }

    on_desklet_added_to_desktop() {
        this._onSettingsChanged();
        global.log( "Desklet Added");
    }

    on_desklet_removed() {
        clearInterval( this._interval);
        global.log( "Desklet Removed");
    }
};

function main( metadata, deskletID)
{
    return new TextualTimeDesklet( metadata, deskletID);
}
