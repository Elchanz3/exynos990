#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <soc/samsung/cal-if.h>

#include "fvmap.h"
#include "cmucal.h"
#include "vclk.h"
#include "ra.h"

#define FVMAP_SIZE		(SZ_8K)
#define STEP_UV			(6250)

void __iomem *fvmap_base;
void __iomem *sram_fvmap_base;

static int init_margin_table[MAX_MARGIN_ID];
static int volt_offset_percent = 0;
static int percent_margin_table[MAX_MARGIN_ID];

static int __init get_mif_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MIF] = volt;

	return 0;
}
early_param("mif", get_mif_volt);

static int __init get_int_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INT] = volt;

	return 0;
}
early_param("int", get_int_volt);

static int __init get_big_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_BIG] = volt;

	return 0;
}
early_param("big", get_big_volt);

static int __init get_mid_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MID] = volt;

	return 0;
}
early_param("mid", get_mid_volt);

static int __init get_lit_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_LIT] = volt;

	return 0;
}
early_param("lit", get_lit_volt);

static int __init get_g3d_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3D] = volt;

	return 0;
}
early_param("g3d", get_g3d_volt);

static int __init get_intcam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INTCAM] = volt;

	return 0;
}
early_param("intcam", get_intcam_volt);

static int __init get_cam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CAM] = volt;

	return 0;
}
early_param("cam", get_cam_volt);

static int __init get_disp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DISP] = volt;

	return 0;
}
early_param("disp", get_disp_volt);

static int __init get_g3dm_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3DM] = volt;

	return 0;
}
early_param("g3dm", get_g3dm_volt);

static int __init get_cp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CP] = volt;

	return 0;
}
early_param("cp", get_cp_volt);

static int __init get_fsys0_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_FSYS0] = volt;

	return 0;
}
early_param("fsys0", get_fsys0_volt);

static int __init get_aud_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_AUD] = volt;

	return 0;
}
early_param("aud", get_aud_volt);

static int __init get_iva_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_IVA] = volt;

	return 0;
}
early_param("iva", get_iva_volt);

static int __init get_score_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_SCORE] = volt;

	return 0;
}
early_param("score", get_score_volt);

static int __init get_npu_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_NPU] = volt;

	return 0;
}
early_param("npu", get_npu_volt);

static int __init get_mfc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MFC] = volt;

	return 0;
}
early_param("mfc", get_mfc_volt);

static int __init get_dsp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DSP] = volt;

	return 0;
}
early_param("dsp", get_dsp_volt);

static int __init get_dnc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DNC] = volt;

	return 0;
}
early_param("dnc", get_dnc_volt);

static int __init get_tnr_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_TNR] = volt;

	return 0;
}
early_param("tnr", get_tnr_volt);

static int __init get_percent_margin_volt(char *str)
{
	int percent;

	get_option(&str, &percent);
	volt_offset_percent = percent;

	return 0;
}
early_param("volt_offset_percent", get_percent_margin_volt);

int fvmap_set_raw_voltage_table(unsigned int id, int uV)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int num_of_lv;
	int idx, i;

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		fv_table->table[i].volt += uV;

	return 0;
}

int fvmap_get_voltage_table(unsigned int id, unsigned int *table)
{
	struct fvmap_header *fvmap_header = fvmap_base;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;

	if (!IS_ACPM_VCLK(id))
		return 0;

	idx = GET_IDX(id);

	fvmap_header = fvmap_base;
	fv_table = fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	return num_of_lv;

}

int fvmap_get_raw_voltage_table(unsigned int id)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;
	unsigned int table[20];

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	for (i = 0; i < num_of_lv; i++)
		printk("dvfs id : %d  %d Khz : %d uv\n", ACPM_VCLK_TYPE | id, fv_table->table[i].rate, table[i]);

	return 0;
}

static void check_percent_margin(struct rate_volt_header *head, unsigned int num_of_lv)
{
	int org_volt;
	int percent_volt;
	int i;

	if (!volt_offset_percent)
		return;

	for (i = 0; i < num_of_lv; i++) {
		org_volt = head->table[i].volt;
		percent_volt = org_volt * volt_offset_percent / 100;
		head->table[i].volt = org_volt + rounddown(percent_volt, STEP_UV);
	}
}

static int get_vclk_id_from_margin_id(int margin_id)
{
	int size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	int i;
	struct vclk *vclk;

	for (i = 0; i < size; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);

		if (vclk->margin_id == margin_id)
			return i;
	}

	return -EINVAL;
}

#define attr_percent(margin_id, type)								\
static ssize_t show_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return snprintf(buf, PAGE_SIZE, "%d\n", percent_margin_table[margin_id]);		\
}												\
												\
static ssize_t store_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)		\
{												\
	int input, vclk_id;									\
												\
	if (!sscanf(buf, "%d", &input))								\
		return -EINVAL;									\
												\
	if (input < -100 || input > 100)							\
		return -EINVAL;									\
												\
	vclk_id = get_vclk_id_from_margin_id(margin_id);					\
	if (vclk_id == -EINVAL)									\
		return vclk_id;									\
	percent_margin_table[margin_id] = input;						\
	cal_dfs_set_volt_margin(vclk_id | ACPM_VCLK_TYPE, input);				\
												\
	return count;										\
}												\
												\
static struct kobj_attribute type##_percent =							\
__ATTR(type##_percent, 0600,									\
	show_##type##_percent, store_##type##_percent)

attr_percent(MARGIN_MIF, mif_margin);
attr_percent(MARGIN_INT, int_margin);
attr_percent(MARGIN_BIG, big_margin);
attr_percent(MARGIN_MID, mid_margin);
attr_percent(MARGIN_LIT, lit_margin);
attr_percent(MARGIN_G3D, g3d_margin);
attr_percent(MARGIN_INTCAM, intcam_margin);
attr_percent(MARGIN_CAM, cam_margin);
attr_percent(MARGIN_DISP, disp_margin);
attr_percent(MARGIN_CP, cp_margin);
attr_percent(MARGIN_FSYS0, fsys0_margin);
attr_percent(MARGIN_AUD, aud_margin);
attr_percent(MARGIN_IVA, iva_margin);
attr_percent(MARGIN_SCORE, score_margin);
attr_percent(MARGIN_NPU, npu_margin);
attr_percent(MARGIN_MFC, mfc_margin);

static struct attribute *percent_margin_attrs[] = {
	&mif_margin_percent.attr,
	&int_margin_percent.attr,
	&big_margin_percent.attr,
	&mid_margin_percent.attr,
	&lit_margin_percent.attr,
	&g3d_margin_percent.attr,
	&intcam_margin_percent.attr,
	&cam_margin_percent.attr,
	&disp_margin_percent.attr,
	&cp_margin_percent.attr,
	&fsys0_margin_percent.attr,
	&aud_margin_percent.attr,
	&iva_margin_percent.attr,
	&score_margin_percent.attr,
	&npu_margin_percent.attr,
	&mfc_margin_percent.attr,
	NULL,
};

static const struct attribute_group percent_margin_group = {
	.attrs = percent_margin_attrs,
};

static void optimize_rate_volt_table(struct rate_volt_header *head, unsigned int num_of_lv) {
	bool changed;
	int i;

	/* optimize voltages */
	while (true) {
		changed = false;

		for (i = 1; i < num_of_lv; i++) {
			/* switch voltages if higher frequency uses less */
			if (head->table[i].volt > head->table[i-1].volt) {
				int temp_volt = head->table[i-1].volt;

				head->table[i-1].volt = head->table[i].volt;
				head->table[i].volt = temp_volt;

				changed = true;
			}
		}

		if (!changed)
			break;
	}
}

#ifdef CONFIG_SEC_FACTORY
#ifdef CONFIG_SOC_EXYNOS9830
#include <soc/samsung/ect_parser.h>
#include <soc/samsung/cal-if.h>
#include "asv.h"

#define VUP_0 12500
#define VUP_1 25000

enum spec_volt_type {
	ASV_G_FUSED = 0,
	ASV_G_GRP
};

static int asv_g_yield_volt[4];

static int __init get_mvup(char *str)
{
	asv_g_yield_volt[1] = VUP_0;

	return 0;
}
early_param("mvup", get_mvup);

static int __init get_bvup(char *str)
{
	asv_g_yield_volt[2] = VUP_1;

	return 0;
}
early_param("bvup", get_bvup);

static int __init get_gvup(char *str)
{
	asv_g_yield_volt[3] = VUP_1;

	return 0;
}
early_param("gvup", get_gvup);

static ssize_t show_asv_g_spec(int id, enum spec_volt_type type, char *buf)
{
	void *gen_block;
	struct ect_gen_param_table *spec;
	int asv_tbl_ver, asv_grp, tbl_size, j, vtyp_freq, num_lv;
	unsigned int fused_volt, grp_volt = 0, volt;
	struct dvfs_rate_volt rate_volt[48];
	unsigned int (*spec_table)[10];
	char *spec_table_name[5] = { "SPEC_CPUCL0", "SPEC_CPUCL1", "SPEC_CPUCL2", "SPEC_G3D" };
	int cal_id[5] = { ACPM_VCLK_TYPE | 2, ACPM_VCLK_TYPE | 3, ACPM_VCLK_TYPE | 4, ACPM_VCLK_TYPE | 5, ACPM_VCLK_TYPE | 10 };
	ssize_t size = 0;

	asv_tbl_ver = asv_get_table_ver();
	if (asv_tbl_ver < 3) {
		pr_err("%s: Do not support ASV-G under ASV table version 3\n", __func__);
		goto out;
	}

	gen_block = ect_get_block("GEN");
	if (gen_block == NULL) {
		pr_err("%s: Failed to get gen block from ECT\n", __func__);
		goto out;
	}

	asv_grp = asv_get_grp(cal_id[id]);
	if (!asv_grp) {
		pr_err("%s: There has no ASV-G information for %s group 0\n", __func__, spec_table_name[id]);
		goto out;
	}

	spec = ect_gen_param_get_table(gen_block, spec_table_name[id]);
	if (spec == NULL) {
		pr_err("%s: Failed to get spec table from ECT\n", __func__);
		goto out;
	}
	spec_table = (unsigned int (*)[10])spec->parameter;

	for (j = 0; j < spec->num_of_row; j++) {
		if (spec_table[j][0] == asv_tbl_ver) {
			grp_volt = spec_table[j][asv_grp + 1];
			vtyp_freq = spec_table[j][1];
			break;
		}
	}

	if (!grp_volt) {
		pr_err("%s: Failed to get grp volt\n", __func__);
		goto out;
	}

	num_lv = cal_dfs_get_lv_num(cal_id[id]);
	tbl_size = cal_dfs_get_rate_asv_table(cal_id[id], rate_volt);
	if (!tbl_size) {
		pr_err("%s: Failed to get asv table\n", __func__);
		goto out;
	}

	for (j = 0; j < num_lv; j++){
		if (rate_volt[j].rate == vtyp_freq) {
			fused_volt = rate_volt[j].volt;
			break;
		}
	}
	if (j == num_lv) {
		pr_err("%s: There has no frequency %d on %d domain\n", __func__, vtyp_freq, spec_table_name[id]);
		goto out;
	}

	fused_volt -= asv_g_yield_volt[id];

	volt = (type == ASV_G_FUSED) ? fused_volt : grp_volt;

	size += snprintf(buf + size, PAGE_SIZE, "%d\n", volt);
out:

	return size;
}

#define asv_g_spec(domain, id)									\
static ssize_t show_asv_g_spec_##domain##_fused_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_FUSED, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_fused_volt =					\
__ATTR(domain##_fused_volt, 0400, show_asv_g_spec_##domain##_fused_volt, NULL);	\
static ssize_t show_asv_g_spec_##domain##_grp_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_GRP, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_grp_volt =					\
__ATTR(domain##_grp_volt, 0400, show_asv_g_spec_##domain##_grp_volt, NULL)

#define asv_g_spec_attr(domain)									\
	&asv_g_spec_##domain##_fused_volt.attr,							\
	&asv_g_spec_##domain##_grp_volt.attr

asv_g_spec(cpucl0, 2);
asv_g_spec(cpucl1, 3);
asv_g_spec(cpucl2, 4);
asv_g_spec(g3d, 5);

static struct attribute *asv_g_spec_attrs[] = {
	asv_g_spec_attr(cpucl0),
	asv_g_spec_attr(cpucl1),
	asv_g_spec_attr(cpucl2),
	asv_g_spec_attr(g3d),
	NULL,
};

static const struct attribute_group asv_g_spec_grp = {
	.attrs = asv_g_spec_attrs,
};
#endif
#endif /* CONFIG_SEC_FACTORY */

ssize_t fvmap_print(char *buf, unsigned int dvfs_type)
{
	volatile struct fvmap_header *fvmap_header;
	struct rate_volt_header *cur;
	int size;
	int i, j;
	ssize_t len = 0;

	fvmap_header = fvmap_base;
	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	for (i = 0; i < size; i++) {
		if (fvmap_header[i].dvfs_type == dvfs_type) {
			cur = fvmap_base + fvmap_header[i].o_ratevolt;

			for (j = 0; j < fvmap_header[i].num_of_lv; j++)
				len += sprintf(buf + len, "%d %d\n",
					cur->table[j].rate, cur->table[j].volt);
		}
	}

	return len;
}

static void fvmap_copy_from_sram(void __iomem *map_base, void __iomem *sram_base)
{
	struct fvmap_header *fvmap_header, *header;
	struct rate_volt_header *old, *new;
	struct clocks *clks;
	struct pll_header *plls;
	struct vclk *vclk;
	unsigned int member_addr;
	unsigned int blk_idx;
	int size, margin;
	int i, j;

	fvmap_header = map_base;
	header = sram_base;

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	for (i = 0; i < size; i++) {
		/* load fvmap info */
		fvmap_header[i].dvfs_type = header[i].dvfs_type;
		fvmap_header[i].num_of_lv = header[i].num_of_lv;
		fvmap_header[i].num_of_members = header[i].num_of_members;
		fvmap_header[i].num_of_pll = header[i].num_of_pll;
		fvmap_header[i].num_of_mux = header[i].num_of_mux;
		fvmap_header[i].num_of_div = header[i].num_of_div;
		fvmap_header[i].gearratio = header[i].gearratio;
		fvmap_header[i].init_lv = header[i].init_lv;
		fvmap_header[i].num_of_gate = header[i].num_of_gate;
		fvmap_header[i].reserved[0] = header[i].reserved[0];
		fvmap_header[i].reserved[1] = header[i].reserved[1];
		fvmap_header[i].block_addr[0] = header[i].block_addr[0];
		fvmap_header[i].block_addr[1] = header[i].block_addr[1];
		fvmap_header[i].block_addr[2] = header[i].block_addr[2];
		fvmap_header[i].o_members = header[i].o_members;
		fvmap_header[i].o_ratevolt = header[i].o_ratevolt;
		fvmap_header[i].o_tables = header[i].o_tables;

		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (vclk == NULL)
			continue;
		pr_info("dvfs_type : %s - id : %x\n",
				vclk->name, fvmap_header[i].dvfs_type);
		pr_info("  num_of_lv      : %d\n", fvmap_header[i].num_of_lv);
		pr_info("  num_of_members : %d\n", fvmap_header[i].num_of_members);

		old = sram_base + fvmap_header[i].o_ratevolt;
		new = map_base + fvmap_header[i].o_ratevolt;
	        
	        optimize_rate_volt_table(old, fvmap_header[i].num_of_lv);

		check_percent_margin(old, fvmap_header[i].num_of_lv);

		margin = init_margin_table[vclk->margin_id];
		if (margin)
			cal_dfs_set_volt_margin(i | ACPM_VCLK_TYPE, margin);

		for (j = 0; j < fvmap_header[i].num_of_members; j++) {
			clks = sram_base + fvmap_header[i].o_members;

			if (j < fvmap_header[i].num_of_pll) {
				plls = sram_base + clks->addr[j];
				member_addr = plls->addr - 0x90000000;
			} else {

				member_addr = (clks->addr[j] & ~0x3) & 0xffff;
				blk_idx = clks->addr[j] & 0x3;

				member_addr |= ((fvmap_header[i].block_addr[blk_idx]) << 16) - 0x90000000;
			}


			vclk->list[j] = cmucal_get_id_by_addr(member_addr);

			if (vclk->list[j] == INVALID_CLK_ID)
				pr_info("  Invalid addr :0x%x\n", member_addr);
			else
				pr_info("  DVFS CMU addr:0x%x\n", member_addr);
		}

		for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
			new->table[j].rate = old->table[j].rate;
			new->table[j].volt = old->table[j].volt;
			pr_info("  lv : [%7d], volt = %d uV (%d %%) \n",
					new->table[j].rate, new->table[j].volt,
					volt_offset_percent);
		}
	}
}

static int patch_fvmap(void __iomem *map_base, unsigned int dvfs_type, unsigned int rate, unsigned int volt)
{
	volatile struct fvmap_header *fvmap_header;
	struct rate_volt_header *rvh;
	bool exists = false;
	int size, rest;
	int i, j, k;
	int ret = 0;

	if (rate < 1)
		return -1;

	fvmap_header = map_base;
	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	if (volt > 10000)
		if ((rest = volt % STEP_UV) != 0) 
				volt += STEP_UV - rest;

	for (i = 0; i < size; i++) {
		if (fvmap_header[i].dvfs_type == dvfs_type) {
			rvh = map_base + fvmap_header[i].o_ratevolt;

			for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
				if (rvh->table[j].rate == rate) {
					exists = true;
					break;
				}
			}

			if (exists) {
				if (volt > 0) {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
						if (rvh->table[j].rate == rate) {
							if (volt > 10000)
								rvh->table[j].volt = volt;
							else
								rvh->table[j].volt += volt * STEP_UV;
							break;
						}
					}
				} else {
					for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
						if (rvh->table[j].rate == rate) {
							for (k = j; k < fvmap_header[i].num_of_lv - 1; k++) {
								rvh->table[k].rate = rvh->table[k + 1].rate;
								rvh->table[k].volt = rvh->table[k + 1].volt;
							}
							fvmap_header[i].num_of_lv--;
							break;
						}
					}
				}
				ret = 1;
			} else {
				int insertPosition = -1;

				for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
					if (rvh->table[j].rate < rate) {
						insertPosition = j;
						break;
					}
				}

				if (insertPosition < 0)
					insertPosition = fvmap_header[i].num_of_lv;

				for (j = fvmap_header[i].num_of_lv; j > insertPosition; j--) {
					rvh->table[j].rate = rvh->table[j - 1].rate;
					rvh->table[j].volt = rvh->table[j - 1].volt;
				}

				rvh->table[insertPosition].rate = rate;
				if (volt > 10000)
					rvh->table[insertPosition].volt = volt;
				else if (volt > 0)
					rvh->table[insertPosition].volt += volt * STEP_UV;

				fvmap_header[i].num_of_lv++;
			}
			break;
		}
	}

	return ret;
}

int fvmap_patch(unsigned int dvfs_type, unsigned int rate, unsigned int volt)
{
	int ret;

	ret = patch_fvmap(fvmap_base, dvfs_type, rate, volt);
	if (ret == 1) {
		ret = 0;
		patch_fvmap(sram_fvmap_base, dvfs_type, rate, volt);
	}

	return ret;
}

static unsigned int read_fvmap(void __iomem *map_base, unsigned int dvfs_type, int mode, unsigned int value)
{
	volatile struct fvmap_header *fvmap_header;
	struct rate_volt_header *rvh;
	int size, rest;
	int i, j;
	unsigned int ret = 0;

	fvmap_header = map_base;
	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	if (mode == READ_RATE)
		if ((rest = value % STEP_UV) != 0) 
				value += STEP_UV - rest;

	for (i = 0; i < size; i++) {
		if (fvmap_header[i].dvfs_type == dvfs_type) {
			rvh = map_base + fvmap_header[i].o_ratevolt;

			for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
				if (mode == READ_VOLT && rvh->table[j].rate == value) {
					ret = rvh->table[j].volt;
					break;
				} else if (mode == READ_RATE && rvh->table[j].volt == value) {
					ret = rvh->table[j].rate;
					break;
				}
			}
		}
	}

	return ret;
}

unsigned int fvmap_read(unsigned int dvfs_type, int mode, unsigned int value)
{
	unsigned int ret;
	
	ret = read_fvmap(fvmap_base, dvfs_type, mode, value);
	if (!ret)
		ret = read_fvmap(sram_fvmap_base, dvfs_type, mode, value);

	return ret;
}
static ssize_t show_patch(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return print_fvmap(buf, 2, 3, 5); /* Only Print CL0/CL1/CL2/G3D to Buffer */
}

static ssize_t store_patch(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	volatile struct fvmap_header *fvmap_header;
	unsigned int dvfs, rate, volt = 0;
	struct vclk *vclk;
	char dvfs_name[63];
	size_t ret;
	int size;
	int i;

	if (sscanf(buf, "%s %u %u", dvfs_name, &rate, &volt) != 3 &&
		sscanf(buf, "%u %u %u", &dvfs, &rate, &volt) != 3 &&
		sscanf(buf, "%s:%u:%u", dvfs_name, &rate, &volt) != 3 &&
		sscanf(buf, "%u:%u:%u", &dvfs, &rate, &volt) != 3)
		return -EINVAL;

	if (!dvfs) {
		fvmap_header = fvmap_base;
		size = cmucal_get_list_size(ACPM_VCLK_TYPE);

		for (i = 0; i < size; i++) {
			vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
			if (vclk == NULL)
				continue;
			if (strstr(vclk->name, dvfs_name) != NULL) {
				dvfs = fvmap_header[i].dvfs_type;
				break;
			}
		}
	}

	if (!dvfs)
		return -EINVAL;

	ret = fvmap_patch(dvfs, rate, volt);
	if (ret)
		return ret;

	return count;
}

static struct kobj_attribute patch =
__ATTR(patch, 0644, show_patch, store_patch);

static struct attribute *fvmap_attrs[] = {
	&patch.attr,
	NULL,
};

static const struct attribute_group fvmap_group = {
	.attrs = fvmap_attrs,
};

int fvmap_init(void __iomem *sram_base)
{
	void __iomem *map_base;
	struct kobject *kobj;
#ifdef CONFIG_SEC_FACTORY
	struct kobject *asv_g_kobj;
#endif /* CONFIG_SEC_FACTORY */

	map_base = kzalloc(FVMAP_SIZE, GFP_KERNEL);

	fvmap_base = map_base;
	sram_fvmap_base = sram_base;
	pr_info("%s:fvmap initialize %p\n", __func__, sram_base);
	fvmap_copy_from_sram(map_base, sram_base);

	/* percent margin for each doamin at runtime */
	kobj = kobject_create_and_add("percent_margin", power_kobj);
	if (!kobj)
		pr_err("Fail to create percent_margin kboject\n");

	if (sysfs_create_group(kobj, &percent_margin_group))
		pr_err("Fail to create percent_margin group\n");

#ifdef CONFIG_SEC_FACTORY
#ifdef CONFIG_SOC_EXYNOS9830
	asv_g_kobj = kobject_create_and_add("asv_g_spec", power_kobj);
	if (sysfs_create_group(asv_g_kobj, &asv_g_spec_grp))
		pr_err("Fail to create asv_g_spec group\n");
#endif
#endif /* CONFIG_SEC_FACTORY */

	return 0;
}
