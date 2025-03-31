unit adc_bl702_config;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, Spin;
const
  ADCshlcount = 3;

type
  TFormAdcConfig = class(TForm)
    RadioGroupAdcChannels: TRadioGroup;
    ButtonOk: TButton;
    ButtonCancel: TButton;
    LabelUz: TLabel;
    LabelIz: TLabel;
    EditUz: TEdit;
    EditIz: TEdit;
    ButtonCopyUz: TButton;
    ButtonCopyIz: TButton;
    SpinEditAdcSmps: TSpinEdit;
    LabelSPS: TLabel;
    EditUk: TEdit;
    Label1: TLabel;
    EditIk: TEdit;
    LabelIk: TLabel;
    LabelSysClk: TLabel;
    procedure FormActivate(Sender: TObject);
    procedure ButtonCopyUzClick(Sender: TObject);
    procedure ButtonCopyIzClick(Sender: TObject);
    procedure ButtonOkClick(Sender: TObject);
    procedure ShowScrParms;
    procedure GetScrParms;
    procedure GetParams;
    procedure SpinEditAdcSmpsChange(Sender: TObject);
//    procedure SetParamIU(i : double ; u : double);
  private
    { Private declarations }
    ChgEna : boolean;
  public
    { Public declarations }
  end;

var
  FormAdcConfig: TFormAdcConfig;

  ADC_channel : byte = 3;
  ADC_smps : dword = 48000;
  ADC_freq : dword = 24576000;

  Uk_adc : double = 0.000000178814;
  Uz_adc : double = 2.5;
  Ik_adc : double = 0.00000178814;
  Iz_adc : double = 0;

  Adc_channels_tab : array [0..ADCshlcount - 1] of byte = (
    $01,  // L channel
    $02,  // R channel
    $03   // LR channel
  );

implementation

{$R *.dfm}
Uses MainFrm;


procedure TFormAdcConfig.GetScrParms;
begin
    DecimalSeparator := '.';
    Uk_adc := StrToFloat(EditUk.Text);
    Uz_adc := StrToFloat(EditUz.Text);
    Ik_adc := StrToFloat(EditIk.Text);
    Iz_adc := StrToFloat(EditIz.Text);

    ADC_channel := Adc_channels_tab[RadioGroupAdcChannels.ItemIndex];
    ADC_smps := SpinEditAdcSmps.Value;
    ADC_freq := ADC_smps * 512;
//    SpinEditAdcSmps.Value := ADC_smps;
end;

procedure TFormAdcConfig.FormActivate(Sender: TObject);
begin
     ChgEna := False;
     ShowScrParms;
     ChgEna := True;
end;

procedure TFormAdcConfig.GetParams;
begin
   U_zero := Uz_adc;
   Uk := Uk_adc;
   I_zero := Iz_adc;
   Ik := Ik_adc;
end;

procedure TFormAdcConfig.ShowScrParms;
var
i : integer;
begin
    DecimalSeparator := '.';
    EditUz.Text:=FormatFloat('0.00000000000', Uz_adc);
    EditUk.Text:=FormatFloat('0.00000000000', Uk_adc);
    EditIz.Text:=FormatFloat('0.00000000000', Iz_adc);
    EditIk.Text:=FormatFloat('0.00000000000', Ik_adc);
    for i:=0 to ADCshlcount do begin
      if ADC_channel = Adc_channels_tab[i] then break;
    end;
    RadioGroupAdcChannels.ItemIndex := i;
    SpinEditAdcSmps.Value := ADC_smps;
    ADC_freq := ADC_smps * 512;
    LabelSysClk.Caption := 'SysClk: ' +  IntToStr(ADC_freq) + ' Hz';
end;

procedure TFormAdcConfig.ButtonCopyUzClick(Sender: TObject);
begin
    EditUz.Text:=FormatFloat('0.00000000', -OldsU);
end;

procedure TFormAdcConfig.ButtonCopyIzClick(Sender: TObject);
begin
    EditIz.Text:=FormatFloat('0.00000000', -OldsI);
end;

procedure TFormAdcConfig.ButtonOkClick(Sender: TObject);
begin
    GetScrParms;
    ModalResult := mrOk;
    Exit;
end;

procedure TFormAdcConfig.SpinEditAdcSmpsChange(Sender: TObject);
var
x : dword;
begin
    x := SpinEditAdcSmps.Value;
    if (x >= 1000) and (x <= 100000) then begin
      ADC_smps := x;
      ADC_freq := x * 512;
      LabelSysClk.Caption := 'SysClk: ' +  IntToStr(ADC_freq) + ' Hz';
    end
    else begin
      LabelSysClk.Caption := 'SysClk: ?';
    end;
end;

end.
